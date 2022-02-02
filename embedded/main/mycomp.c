#include "mycomp.h"
#include "esp_log.h"
#include "globals.h"

static const char *TAG = "MYCOMP";

typedef struct {
    int max_sample;
    unsigned char **inbuf;
    unsigned char *outbuf;
    void *downmix_handle;
    int* ticks_to_wait;
    int reset_flag;
} mycomp_t;

static IIR_FILTER h1 = {.taps = {{0.000008, 0.000016, 0.000008, 1.000000, -1.998173, 0.998206},
                               {0.000008, 0.000016, 0.000008, 1.000000, -1.994770, 0.994803},
                               {0.000008, 0.000016, 0.000008, 1.000000, -1.991884, 0.991917},
                               {0.000008, 0.000016, 0.000008, 1.000000, -1.989792, 0.989825},
                               {0.000008, 0.000016, 0.000008, 1.000000, -1.988694, 0.988727}}};

static IIR_FILTER h2 = {.taps = {{0.002913, 0.000000, -0.002913, 1.000000, -1.997491, 0.997623},
                               {0.002913, 0.000000, -0.002913, 1.000000, -1.998741, 0.998776},
                               {0.002909, 0.000000, -0.002909, 1.000000, -1.994195, 0.994300},
                               {0.002909, 0.000000, -0.002909, 1.000000, -1.996243, 0.996287},
                               {0.002908, 0.000000, -0.002908, 1.000000, -1.994117, 0.994185}}};

static IIR_FILTER h3 = {.taps = {{0.005776, 0.000000, -0.005776, 1.000000, -1.994771, 0.995290},
                               {0.005776, 0.000000, -0.005776, 1.000000, -1.997435, 0.997572},
                               {0.005759, 0.000000, -0.005759, 1.000000, -1.988313, 0.988722},
                               {0.005759, 0.000000, -0.005759, 1.000000, -1.992472, 0.992646},
                               {0.005753, 0.000000, -0.005753, 1.000000, -1.988228, 0.988495}}};

static IIR_FILTER h4 = {.taps = {{0.011530, 0.000000, -0.011530, 1.000000, -1.988533, 0.990603},
                               {0.011530, 0.000000, -0.011530, 1.000000, -1.994600, 0.995150},
                               {0.011465, 0.000000, -0.011465, 1.000000, -1.975944, 0.977571},
                               {0.011465, 0.000000, -0.011465, 1.000000, -1.984653, 0.985345},
                               {0.011440, 0.000000, -0.011440, 1.000000, -1.976062, 0.977120}}};

static IIR_FILTER h5 = {.taps = {{0.022978, 0.000000, -0.022978, 1.000000, -1.973066, 0.981304},
                               {0.022978, 0.000000, -0.022978, 1.000000, -1.988127, 0.990322},
                               {0.022720, 0.000000, -0.022720, 1.000000, -1.949213, 0.955647},
                               {0.022720, 0.000000, -0.022720, 1.000000, -1.968149, 0.970896},
                               {0.022624, 0.000000, -0.022624, 1.000000, -1.950566, 0.954752}}};

static IIR_FILTER h6 = {.taps = {{0.045623, 0.000000, -0.045623, 1.000000, -1.930444, 0.963025},
                               {0.045623, 0.000000, -0.045623, 1.000000, -1.971986, 0.980716},
                               {0.044632, 0.000000, -0.044632, 1.000000, -1.888132, 0.913282},
                               {0.044632, 0.000000, -0.044632, 1.000000, -1.931749, 0.942572},
                               {0.044270, 0.000000, -0.044270, 1.000000, -1.895096, 0.911461}}};

static IIR_FILTER h7 = {.taps = {{0.089920, 0.000000, -0.089920, 1.000000, -1.800973, 0.927945},
                               {0.089920, 0.000000, -0.089920, 1.000000, -1.927131, 0.961639},
                               {0.086242, 0.000000, -0.086242, 1.000000, -1.738240, 0.834239},
                               {0.086242, 0.000000, -0.086242, 1.000000, -1.845942, 0.887928},
                               {0.084952, 0.000000, -0.084952, 1.000000, -1.767558, 0.830095}}};

static IIR_FILTER h8 = {.taps = {{0.174589, 0.000000, -0.174589, 1.000000, -1.389359, 0.865057},
                               {0.174589, 0.000000, -0.174589, 1.000000, -1.789258, 0.923448},
                               {0.161910, 0.000000, -0.161910, 1.000000, -1.627206, 0.784649},
                               {0.161910, 0.000000, -0.161910, 1.000000, -1.348507, 0.697201},
                               {0.157748, 0.000000, -0.157748, 1.000000, -1.456221, 0.684504}}};

static IIR_FILTER h9 = {.taps = {{0.329196, 0.000000, -0.329196, 1.000000, -1.345449, 0.841947},
                               {0.329196, 0.000000, -0.329196, 1.000000, -0.188873, 0.777890},
                               {0.291067, 0.000000, -0.291067, 1.000000, -1.047034, 0.588664},
                               {0.291067, 0.000000, -0.291067, 1.000000, -0.360327, 0.499053},
                               {0.279645, 0.000000, -0.279645, 1.000000, -0.686184, 0.440711}}};

static IIR_FILTER h10 = {.taps = {{0.590011, 0.000000, -0.590011, 1.000000, 1.877351, 0.908749},
                               {0.590011, 0.000000, -0.590011, 1.000000, -0.123640, 0.582463},
                               {0.502106, 0.000000, -0.502106, 1.000000, 1.708580, 0.742893},
                               {0.502106, 0.000000, -0.502106, 1.000000, 0.014155, 0.144786},
                               {0.477582, 0.000000 -0.,477582, 1.000000, 0.857492, 0.044836}}};

static IIR_FILTER j1 = {.taps = {{0.000008, 0.000016, 0.000008, 1.000000, -1.998173, 0.998206},
                               {0.000008, 0.000016, 0.000008, 1.000000, -1.994770, 0.994803},
                               {0.000008, 0.000016, 0.000008, 1.000000, -1.991884, 0.991917},
                               {0.000008, 0.000016, 0.000008, 1.000000, -1.989792, 0.989825},
                               {0.000008, 0.000016, 0.000008, 1.000000, -1.988694, 0.988727}}};

static IIR_FILTER j2 = {.taps = {{0.002913, 0.000000, -0.002913, 1.000000, -1.997491, 0.997623},
                               {0.002913, 0.000000, -0.002913, 1.000000, -1.998741, 0.998776},
                               {0.002909, 0.000000, -0.002909, 1.000000, -1.994195, 0.994300},
                               {0.002909, 0.000000, -0.002909, 1.000000, -1.996243, 0.996287},
                               {0.002908, 0.000000, -0.002908, 1.000000, -1.994117, 0.994185}}};

static IIR_FILTER j3 = {.taps = {{0.005776, 0.000000, -0.005776, 1.000000, -1.994771, 0.995290},
                               {0.005776, 0.000000, -0.005776, 1.000000, -1.997435, 0.997572},
                               {0.005759, 0.000000, -0.005759, 1.000000, -1.988313, 0.988722},
                               {0.005759, 0.000000, -0.005759, 1.000000, -1.992472, 0.992646},
                               {0.005753, 0.000000, -0.005753, 1.000000, -1.988228, 0.988495}}};

static IIR_FILTER j4 = {.taps = {{0.011530, 0.000000, -0.011530, 1.000000, -1.988533, 0.990603},
                               {0.011530, 0.000000, -0.011530, 1.000000, -1.994600, 0.995150},
                               {0.011465, 0.000000, -0.011465, 1.000000, -1.975944, 0.977571},
                               {0.011465, 0.000000, -0.011465, 1.000000, -1.984653, 0.985345},
                               {0.011440, 0.000000, -0.011440, 1.000000, -1.976062, 0.977120}}};

static IIR_FILTER j5 = {.taps = {{0.022978, 0.000000, -0.022978, 1.000000, -1.973066, 0.981304},
                               {0.022978, 0.000000, -0.022978, 1.000000, -1.988127, 0.990322},
                               {0.022720, 0.000000, -0.022720, 1.000000, -1.949213, 0.955647},
                               {0.022720, 0.000000, -0.022720, 1.000000, -1.968149, 0.970896},
                               {0.022624, 0.000000, -0.022624, 1.000000, -1.950566, 0.954752}}};

static IIR_FILTER j6 = {.taps = {{0.045623, 0.000000, -0.045623, 1.000000, -1.930444, 0.963025},
                               {0.045623, 0.000000, -0.045623, 1.000000, -1.971986, 0.980716},
                               {0.044632, 0.000000, -0.044632, 1.000000, -1.888132, 0.913282},
                               {0.044632, 0.000000, -0.044632, 1.000000, -1.931749, 0.942572},
                               {0.044270, 0.000000, -0.044270, 1.000000, -1.895096, 0.911461}}};

static IIR_FILTER j7 = {.taps = {{0.089920, 0.000000, -0.089920, 1.000000, -1.800973, 0.927945},
                               {0.089920, 0.000000, -0.089920, 1.000000, -1.927131, 0.961639},
                               {0.086242, 0.000000, -0.086242, 1.000000, -1.738240, 0.834239},
                               {0.086242, 0.000000, -0.086242, 1.000000, -1.845942, 0.887928},
                               {0.084952, 0.000000, -0.084952, 1.000000, -1.767558, 0.830095}}};

static IIR_FILTER j8 = {.taps = {{0.174589, 0.000000, -0.174589, 1.000000, -1.389359, 0.865057},
                               {0.174589, 0.000000, -0.174589, 1.000000, -1.789258, 0.923448},
                               {0.161910, 0.000000, -0.161910, 1.000000, -1.627206, 0.784649},
                               {0.161910, 0.000000, -0.161910, 1.000000, -1.348507, 0.697201},
                               {0.157748, 0.000000, -0.157748, 1.000000, -1.456221, 0.684504}}};

static IIR_FILTER j9 = {.taps = {{0.329196, 0.000000, -0.329196, 1.000000, -1.345449, 0.841947},
                               {0.329196, 0.000000, -0.329196, 1.000000, -0.188873, 0.777890},
                               {0.291067, 0.000000, -0.291067, 1.000000, -1.047034, 0.588664},
                               {0.291067, 0.000000, -0.291067, 1.000000, -0.360327, 0.499053},
                               {0.279645, 0.000000, -0.279645, 1.000000, -0.686184, 0.440711}}};

static IIR_FILTER j10 = {.taps = {{0.590011, 0.000000, -0.590011, 1.000000, 1.877351, 0.908749},
                               {0.590011, 0.000000, -0.590011, 1.000000, -0.123640, 0.582463},
                               {0.502106, 0.000000, -0.502106, 1.000000, 1.708580, 0.742893},
                               {0.502106, 0.000000, -0.502106, 1.000000, 0.014155, 0.144786},
                               {0.477582, 0.000000 -0.,477582, 1.000000, 0.857492, 0.044836}}};

static inline float directform_I_process(IIR_FILTER* filter, float sample) {
    float cascaded_sample = sample;

    for (uint16_t stage_num = 0; stage_num < IIR_FILTER_NUM_STAGES; stage_num++) {
        // [X1 X2 Y1 Y2]
        float* regs = filter->buffer[stage_num];

        // [B0 B1 B2 1 A1 A2]
        const float* taps = filter->taps[stage_num];

        // calculate the feed forward using the B terms:
        // (X0 * B0) + (X1 * B1) + (X2 * B2)
        float feed_forward =
            cascaded_sample * taps[0] + regs[0] * taps[1] + regs[1] * taps[2];
        // calculate the feedback using the A terms: (Y1 * -A1) + (Y2 * -A2)
        float feedback = regs[2] * -taps[4] + regs[3] * -taps[5];
        // output is feed forward terms + feedback terms (A0 is always 1, so the
        // output is not getting scaled)
        float output = feed_forward + feedback;

        regs[1] = regs[0];          // X2 = X1
        regs[0] = cascaded_sample;  // X1 = X0

        regs[3] = regs[2];  // Y2 = Y1
        regs[2] = output;   // Y1 = output

        // feed output of filter into other filter
        cascaded_sample = output;
    }

    return cascaded_sample;
}

audio_element_handle_t mycomp_init() {
    audio_element_cfg_t cfg = DEFAULT_AUDIO_ELEMENT_CONFIG();
    cfg.open = mycomp_open;
    cfg.process = mycomp_process;
    cfg.close = mycomp_close;
    cfg.destroy = mycomp_destroy;
    cfg.tag = "mycomp";
    cfg.task_stack = 4 * 1024;
    cfg.task_prio = 5;
    cfg.task_core = 1;
    cfg.out_rb_size = 8 * 2048;
    // cfg.stack_in_ext = config->stack_in_ext;
    // cfg.multi_in_rb_num = config->downmix_info.source_num;
 
    return audio_element_init(&cfg);
}

esp_err_t mycomp_open(audio_element_handle_t self) {
    ESP_LOGI(TAG, "open");
    return ESP_OK;
}

int mycomp_process(audio_element_handle_t self, char *in_buffer, int in_len) {
    int rsize = audio_element_input(self, in_buffer, in_len);

    if (in_len != rsize || (rsize % 4) != 0) {
        ESP_LOGW(TAG, "unexpected rsize: %d, in_len: %d", rsize, in_len);
    }

    int16_t left_sample, right_sample;
    float left_sample_f, right_sample_f;

    float output_left_sample_f, output_right_sample_f;

    char * left_sample_p = (char *) &left_sample;
    char * right_sample_p = (char *) &right_sample;

    for (int i =0; i<rsize; i+=4) {
        right_sample_p[0] = in_buffer[i];
        right_sample_p[1] = in_buffer[i+1];

        left_sample_p[0] = in_buffer[i+2];
        left_sample_p[1] = in_buffer[i+3];

        left_sample_f = (float)left_sample;
        right_sample_f = (float)right_sample;

        output_left_sample_f = directform_I_process(&h1, left_sample_f) * g_equalizer_gain_ratios[0];
        output_left_sample_f += directform_I_process(&h2, left_sample_f) * g_equalizer_gain_ratios[1];
        output_left_sample_f += directform_I_process(&h3, left_sample_f) * g_equalizer_gain_ratios[2];
        output_left_sample_f += directform_I_process(&h4, left_sample_f) * g_equalizer_gain_ratios[3];
        output_left_sample_f += directform_I_process(&h5, left_sample_f) * g_equalizer_gain_ratios[4];
        output_left_sample_f += directform_I_process(&h6, left_sample_f) * g_equalizer_gain_ratios[5];
        output_left_sample_f += directform_I_process(&h7, left_sample_f) * g_equalizer_gain_ratios[6];
        output_left_sample_f += directform_I_process(&h8, left_sample_f) * g_equalizer_gain_ratios[7];
        output_left_sample_f += directform_I_process(&h9, left_sample_f) * g_equalizer_gain_ratios[8];
        // output_left_sample_f += directform_I_process(&h10, left_sample_f) * g_equalizer_gain_ratios[9];

        output_right_sample_f = directform_I_process(&j1, right_sample_f) * g_equalizer_gain_ratios[0];
        output_right_sample_f += directform_I_process(&j2, right_sample_f) * g_equalizer_gain_ratios[1];
        output_right_sample_f += directform_I_process(&j3, right_sample_f) * g_equalizer_gain_ratios[2];
        output_right_sample_f += directform_I_process(&j4, right_sample_f) * g_equalizer_gain_ratios[3];
        output_right_sample_f += directform_I_process(&j5, right_sample_f) * g_equalizer_gain_ratios[4];
        output_right_sample_f += directform_I_process(&j6, right_sample_f) * g_equalizer_gain_ratios[5];
        output_right_sample_f += directform_I_process(&j7, right_sample_f) * g_equalizer_gain_ratios[6];
        output_right_sample_f += directform_I_process(&j8, right_sample_f) * g_equalizer_gain_ratios[7];
        output_right_sample_f += directform_I_process(&j9, right_sample_f) * g_equalizer_gain_ratios[8];
        // output_right_sample_f += directform_I_process(&h10, right_sample_f) * g_equalizer_gain_ratios[9];

        left_sample = (int16_t)output_left_sample_f;
        right_sample = (int16_t)output_right_sample_f;

        in_buffer[i] = right_sample_p[0];
        in_buffer[i+1] = right_sample_p[1];

        in_buffer[i+2] = left_sample_p[0];
        in_buffer[i+3] = left_sample_p[1];
    }

    rsize = audio_element_output(self, in_buffer, rsize);

    return rsize;
}

esp_err_t mycomp_close(audio_element_handle_t self) {
    ESP_LOGI(TAG, "close");
    return ESP_OK;
}

esp_err_t mycomp_destroy(audio_element_handle_t self) {
    ESP_LOGI(TAG, "destroy");
    return ESP_OK;
}