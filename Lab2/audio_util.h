#include <libavcodec/avcodec.h>
#include <libavformat/avformat.h>
#include <stdbool.h>

typedef struct {
	AVFormatContext *format_context;
	AVCodecContext *codec_context;
	const AVCodec *codec;
	AVPacket *packet;
	AVFrame *frame;
	float *samples;
	int32_t samples_cnt;
} AudioInfo;

void init_audio_info(AudioInfo *channel);

void free_samples_mem(float **samples);

void free_resources(AudioInfo channel);

int open_and_find_stream_info(const char *filepath, AVFormatContext **format_context);

int audio_stream_index(AVFormatContext *format_context, int start_idx);

int sample_rate(AVFormatContext *format_context, int index);

int process_audio_stream(AudioInfo *audio_info, int audio_stream_idx);

int decode_into_samples(AudioInfo *channel1, AudioInfo *channel2, int audio_stream_idx_1, int audio_stream_idx_2, int channel_1_idx, int channel_2_idx, int argc);

int find_max_index(const float *array, int size);

int cross_correlation(const float *data1, const float *data2, int len1, int len2, float **correlation);

int resample(const float *input_samples, int real_sample_rate, int target_sample_rate, float **output_samples, int32_t samples_cnt);
