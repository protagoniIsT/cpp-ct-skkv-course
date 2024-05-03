#include "return_codes.h"
#include "audio_util.h"
#include <fftw3.h>
#include <math.h>
#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {

    if (argc < 2) {
        fprintf(stderr, "No files provided");
        return ERROR_ARGUMENTS_INVALID;
    }

    if (argc > 3) {
        fprintf(stderr, "Too many arguments given. Must be 1 or 2");
        return ERROR_ARGUMENTS_INVALID;
    }

    AudioInfo channel1;
    AudioInfo channel2;

    init_audio_info(&channel1);
    init_audio_info(&channel2);

    av_log_set_level(AV_LOG_QUIET);

    int32_t channel_1_idx = 0;
    int32_t channel_2_idx = 0;

    int audio_stream_idx_1 = -1;
    int audio_stream_idx_2 = -1;

    float *correlation = NULL;

	const char *file1 = argv[1];


	int ret1 = open_and_find_stream_info(file1, &channel1.format_context);
	if (ret1 != SUCCESS) return ret1;
	audio_stream_idx_1 = audio_stream_index(channel1.format_context, 0);
	if (audio_stream_idx_1 == -1) {
		fprintf(stderr, "No audio streams found in file 1");
		return ERROR_DATA_INVALID;
	}

	if (argc == 2) {
		int num_of_channels = channel1.format_context->streams[audio_stream_idx_1]->codecpar->ch_layout.nb_channels;
		if (num_of_channels != 2) {
			fprintf(stderr, "Invalid number of channels: %d in file '%s'", num_of_channels, file1);
			return ERROR_FORMAT_INVALID;
		}
		channel_1_idx = 0;
		channel_2_idx = 1;

		int ret_process = process_audio_stream(&channel1, audio_stream_idx_1);
		if (ret_process != SUCCESS) {
			free_resources(channel1);
			return ret_process;
		}

		int ret_dec = decode_into_samples(&channel1, &channel2, audio_stream_idx_1, audio_stream_idx_1, channel_1_idx, channel_2_idx, argc);
		if (ret_dec != SUCCESS) {
			free_resources(channel1);
			return ret_dec;
		}
	} else {
        const char *file2 = argv[2];
        int ret2 = open_and_find_stream_info(file2, &channel2.format_context);
        if (ret2 != SUCCESS) return ret2;

        audio_stream_idx_2 = audio_stream_index(channel2.format_context, 0);
        if (audio_stream_idx_2 == -1) {
            fprintf(stderr, "No audio streams found in file 2");
            return ERROR_DATA_INVALID;
        }

        int ret_process1 = process_audio_stream(&channel1, audio_stream_idx_1);
        if (ret_process1 != SUCCESS) {
			free_resources(channel1);
			return ret_process1;
		}

        int ret_process2 = process_audio_stream(&channel2, audio_stream_idx_2);
        if (ret_process2 != SUCCESS) {
			free_resources(channel1);
			free_resources(channel2);
			return ret_process2;
		}

        int ret_dec = decode_into_samples(&channel1, &channel2, audio_stream_idx_1, audio_stream_idx_2, channel_1_idx, channel_2_idx, argc);
        if (ret_dec != SUCCESS) return ret_dec;
    }

    int sample_rate_total = sample_rate(channel1.format_context, audio_stream_idx_1);

    if (argc == 3) {
        float *resampled = NULL;
        int sample_rate_ch1 = sample_rate(channel1.format_context, audio_stream_idx_1);
        int sample_rate_ch2 = sample_rate(channel2.format_context, audio_stream_idx_2);
        if (sample_rate_ch1 != sample_rate_ch2) {
            bool cond = sample_rate_ch1 > sample_rate_ch2;
            int ret = resample(cond ? channel2.samples : channel1.samples,
                               cond ? sample_rate_ch2 : sample_rate_ch1,
                               cond ? sample_rate_ch1 : sample_rate_ch2,
                               &resampled, cond ? channel2.samples_cnt : channel1.samples_cnt);
            if (ret != SUCCESS) {
                return ret;
            }
            if (cond) {
                free_samples_mem(&channel2.samples);
                channel2.samples = resampled;
            } else {
                free_samples_mem(&channel1.samples);
                channel1.samples = resampled;
                sample_rate_total = sample_rate_ch2;
            }
        }
    }

	int ret_corr = SUCCESS;
    if (channel1.samples_cnt > 0 && channel2.samples_cnt > 0) {
        int N = channel1.samples_cnt + channel2.samples_cnt - 1;
        ret_corr = cross_correlation(channel1.samples, channel2.samples, channel1.samples_cnt, channel2.samples_cnt, &correlation);
        if (ret_corr != SUCCESS) goto cleanup;
        int time_delay_samples = -channel2.samples_cnt + 1 + find_max_index(correlation, N);
		free(correlation);
        double time_delay_ms = (double)time_delay_samples * 1000.0 / sample_rate_total;
        printf("delta: %i samples\nsample rate: %i Hz\ndelta time: %i ms\n", time_delay_samples, sample_rate_total, (int)floor(time_delay_ms));
    }
	cleanup:
	free_resources(channel1);
	free_resources(channel2);
    return ret_corr;
}
