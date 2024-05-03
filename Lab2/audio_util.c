#include "audio_util.h"
#include "return_codes.h"
#include <fftw3.h>
#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <libswresample/swresample.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>

void init_audio_info(AudioInfo *channel) {
	memset(channel, 0, sizeof(AudioInfo));
}

void free_samples_mem(float **samples) {
	if (*samples) {
		free(*samples);
		*samples = NULL;
	}
}

void free_resources(AudioInfo channel) {
	av_frame_free(&channel.frame);
	av_packet_free(&channel.packet);
	avcodec_free_context(&channel.codec_context);
	avformat_close_input(&channel.format_context);
	free(channel.samples);
}

int open_and_find_stream_info(const char *filepath, AVFormatContext **format_context) {
	int ret = avformat_open_input(format_context, filepath, NULL, NULL);
	switch (ret) {
		case AVERROR(EINVAL):
			fprintf(stderr, "Cannot open file: Invalid arguments\n");
			return ERROR_ARGUMENTS_INVALID;
		case AVERROR(ENOMEM):
			fprintf(stderr, "Cannot open file: Not enough memory to process file\n");
			return ERROR_NOTENOUGH_MEMORY;
		case AVERROR_EOF:
			fprintf(stderr, "Cannot open file: EOF occured\n");
			return ERROR_CANNOT_OPEN_FILE;
		case AVERROR(EIO):
			fprintf(stderr, "Cannot open file: Error while reading file\n");
			return ERROR_CANNOT_OPEN_FILE;
		case AVERROR(ENOSYS):
			fprintf(stderr, "Cannot open file: Unsupported functionality\n");
			return ERROR_UNSUPPORTED;
		case AVERROR_EXIT:
			fprintf(stderr, "Cannot open file: Processing was forcibly terminated\n");
			return ERROR_UNKNOWN;
		default: break;
	}
	if (avformat_find_stream_info(*format_context, NULL) < 0) {
		fprintf(stderr, "Could not find stream information in file %s\n", filepath);
		avformat_close_input(format_context);
		return ERROR_FORMAT_INVALID;
	}
	return SUCCESS;
}

int audio_stream_index(AVFormatContext *format_context, int start_idx) {
	for (unsigned int i = start_idx; i < format_context->nb_streams; i++) {
		if (format_context->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO) {
			return (int)i;
		}
	}
	return -1;
}

int sample_rate(AVFormatContext *format_context, int index) {
	return format_context->streams[index]->codecpar->sample_rate;
}

int decode_audio(AVCodecContext *codec_context, AVPacket *packet, AVFrame *frame, float **samples_arr, int32_t *samples_cnt, int32_t channel_num) {
	int send_ret = avcodec_send_packet(codec_context, packet);
	if (send_ret < 0) {
		fprintf(stderr, "Error submitting packet to the decoder\n");
		return ERROR_UNKNOWN;
	}
	size_t curr_capacity = 0;
	while (true) {
		send_ret = avcodec_receive_frame(codec_context, frame);
		if (send_ret == AVERROR(EAGAIN)) {
			break;
		} else if (send_ret == AVERROR_EOF) {
			return SUCCESS;
		} else if (send_ret < 0) {
			fprintf(stderr, "Error during receiving frame\n");
			return ERROR_FORMAT_INVALID;
		}
		size_t required_size = (*samples_cnt + frame->nb_samples) * sizeof(float);
		if (required_size > curr_capacity) {
			size_t new_capacity = required_size * 2;
			float *resized_samples = realloc(*samples_arr, required_size * 2);
			if (resized_samples == NULL) {
				free(samples_arr);
				fprintf(stderr, "Failed to allocate memory for samples while decoding\n");
				return ERROR_NOTENOUGH_MEMORY;
			}
			*samples_arr = resized_samples;
			curr_capacity = new_capacity;
		}
		memcpy(*samples_arr + *samples_cnt, frame->data[channel_num], frame->nb_samples * sizeof(float));
		*samples_cnt += frame->nb_samples;
	}
	return SUCCESS;
}

bool is_supported_codec(unsigned int codec) {
	return (codec == AV_CODEC_ID_FLAC
	     || codec == AV_CODEC_ID_MP2
	     || codec == AV_CODEC_ID_MP3
	     || codec == AV_CODEC_ID_OPUS
	     || codec == AV_CODEC_ID_AAC);
}

int process_audio_stream(AudioInfo *audio_info, int audio_stream_idx) {
	unsigned int codec = audio_info->format_context->streams[audio_stream_idx]->codecpar->codec_id;
	if (!is_supported_codec(codec)) {
		fprintf(stderr, "Unsupported audio codec\n");
		return ERROR_UNSUPPORTED;
	}
	audio_info->codec = avcodec_find_decoder(codec);
	if (!audio_info->codec) {
		fprintf(stderr, "Codec not found\n");
		return ERROR_DATA_INVALID;
	}
	audio_info->codec_context = avcodec_alloc_context3(audio_info->codec);
	if (!audio_info->codec_context) {
		fprintf(stderr, "Could not allocate memory for codec_context\n");
		return ERROR_NOTENOUGH_MEMORY;
	}
	avcodec_parameters_to_context(audio_info->codec_context, audio_info->format_context->streams[audio_stream_idx]->codecpar);
	if (avcodec_open2(audio_info->codec_context, audio_info->codec, NULL) < 0) {
		free_resources(*audio_info);
		fprintf(stderr, "Could not open codec\n");
		return ERROR_NOTENOUGH_MEMORY;
	}
	audio_info->frame = av_frame_alloc();
	audio_info->packet = av_packet_alloc();
	if (!audio_info->frame || !audio_info->packet) {
		free_resources(*audio_info);
		fprintf(stderr, "Could not allocate memory for frame/packet\n");
		return ERROR_NOTENOUGH_MEMORY;
	}
	return SUCCESS;
}

int decode_into_samples(AudioInfo *channel1, AudioInfo *channel2, int audio_stream_idx_1, int audio_stream_idx_2, int channel_1_idx, int channel_2_idx, int argc) {
	bool cond = argc == 2;
	while (av_read_frame(channel1->format_context, channel1->packet) >= 0) {
		if (channel1->packet->stream_index == audio_stream_idx_1) {
			if (decode_audio(channel1->codec_context, channel1->packet, channel1->frame,
							 &channel1->samples, &channel1->samples_cnt, channel_1_idx) != SUCCESS) {
				av_packet_unref(channel1->packet);
				fprintf(stderr, "Error while decoding file\n");
				return ERROR_DATA_INVALID;
			}
			if (cond) {
				if (decode_audio(channel1->codec_context, channel1->packet, channel1->frame,
								 &channel2->samples, &channel2->samples_cnt, channel_2_idx) != SUCCESS) {
					av_packet_unref(channel1->packet);
					fprintf(stderr, "Error while decoding file\n");
					return ERROR_DATA_INVALID;
				}
			}
		}
		av_packet_unref(channel1->packet);
	}
	if (cond) return SUCCESS;
	while (av_read_frame(channel2->format_context, channel2->packet) >= 0) {
		if (channel2->packet->stream_index == audio_stream_idx_2) {
			if (decode_audio(channel2->codec_context, channel2->packet, channel2->frame,
							 &channel2->samples, &channel2->samples_cnt, channel_2_idx) != SUCCESS) {
				av_packet_unref(channel2->packet);
				fprintf(stderr, "Error while decoding file\n");
				return ERROR_DATA_INVALID;
			}
		}
		av_packet_unref(channel2->packet);
	}
	return SUCCESS;
}

int find_max_index(const float *array, int size) {
	int max_index = 0;
	for (int i = 1; i < size; i++) {
		if (array[i] > array[max_index]) {
			max_index = i;
		}
	}
	return max_index;
}

int next_deg(int len) {
	len--;
	len |= len >> 1;
	len |= len >> 2;
	len |= len >> 4;
	len |= len >> 8;
	len |= len >> 16;
	len++;
	return len;
}

int cross_correlation(const float *data1, const float *data2, int len1, int len2, float **correlation) {
	int len_combined = next_deg(len1 + len2 - 1);
	fftw_complex *memory_block;
	size_t block_size = sizeof(fftw_complex) * len_combined;
	int block_count = 5;
	memory_block = (fftw_complex*)fftw_malloc(block_size * block_count);
	if (!memory_block) {
		fprintf(stderr, "Failed to allocate memory for FFTW complex arrays\n");
		return ERROR_NOTENOUGH_MEMORY;
	}

	fftw_complex *complex_in1 = memory_block;
	fftw_complex *complex_in2 = memory_block + len_combined;
	fftw_complex *complex_out1 = memory_block + 2 * len_combined;
	fftw_complex *complex_out2 = memory_block + 3 * len_combined;
	fftw_complex *result = memory_block + 4 * len_combined;

	*correlation = (float*)malloc(sizeof(float) * len_combined);
	if (!*correlation) {
		fftw_free(memory_block);
		fprintf(stderr, "Failed to allocate memory for correlation array\n");
		return ERROR_NOTENOUGH_MEMORY;
	}

	memset(complex_in1, 0, block_size * 2);
	memset(complex_out1, 0, block_size * 2);
	memset(result, 0, block_size);

	for (int i = 0; i < len1; i++) {
		complex_in1[i + len2 - 1][0] = data1[i];
	}
	for (int i = 0; i < len2; i++) {
		complex_in2[i][0] = data2[i];
	}

	fftw_plan plan_forward_1 = fftw_plan_dft_1d(len_combined, complex_in1, complex_out1, FFTW_FORWARD, FFTW_ESTIMATE);
	fftw_plan plan_forward_2 = fftw_plan_dft_1d(len_combined, complex_in2, complex_out2, FFTW_FORWARD, FFTW_ESTIMATE);
	fftw_plan plan_backward = fftw_plan_dft_1d(len_combined, result, complex_in1, FFTW_BACKWARD, FFTW_ESTIMATE);

	fftw_execute(plan_forward_1);
	fftw_execute(plan_forward_2);

	for (int i = 0; i < len_combined; i++) {
		result[i][0] = complex_out1[i][0] * complex_out2[i][0] + complex_out1[i][1] * complex_out2[i][1];
		result[i][1] = complex_out1[i][1] * complex_out2[i][0] - complex_out1[i][0] * complex_out2[i][1];
	}

	fftw_execute(plan_backward);

	for (int i = 0; i < len_combined; i++) {
		(*correlation)[i] = (float)complex_in1[i][0];
	}

	fftw_destroy_plan(plan_forward_1);
	fftw_destroy_plan(plan_forward_2);
	fftw_destroy_plan(plan_backward);
	fftw_free(memory_block);

	return SUCCESS;
}

int resample(const float *input_samples, int real_sample_rate, int target_sample_rate, float **output_samples, int32_t samples_cnt) {
	float ratio = (float)real_sample_rate / (float)target_sample_rate;
	int output_cnt = (int)((float)samples_cnt / ratio);
	*output_samples = malloc(output_cnt * sizeof(float));
	if (!*output_samples) {
		fprintf(stderr, "Memory allocation for resampled array failed\n");
		return ERROR_NOTENOUGH_MEMORY;
	}
	for (int i = 0; i < output_cnt; i++) {
		float float_idx = (float)i * ratio;
		int index = (int)roundf(float_idx);
		float diff = float_idx - (float)index;
		if (index + 1 < samples_cnt) {
			(*output_samples)[i] = input_samples[index] * (1 - diff) + input_samples[index + 1] * diff;
		} else {
			(*output_samples)[i] = input_samples[samples_cnt - 1];
		}
	}
	return SUCCESS;
}

