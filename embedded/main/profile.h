#ifndef PROFILE_H_
#define PROFILE_H_

#include "esp_littlefs.h"

#include "equalizer2.h"
#include "mixer2.h"

#define MAX_NUM_PROFILES 5
#define MAX_PROFILE_NAME_LEN 32

typedef struct profile {
    char name[MAX_PROFILE_NAME_LEN];
    equalizer2_profile equalizer;
    mixer2_profile mixer;
} profile;

extern size_t g_current_profile;
extern profile* g_profiles[MAX_NUM_PROFILES];

#define DEFAULT_PROFILE()\
    {\
        .name = "Profile",\
        .equalizer = {\
            .gains = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}, \
            .enabled = false, \
        },\
        .mixer = {\
            .num_sources = MIXER2_NUM_SOURCES,\
            .settings = {\
                .gains = default_gains2,\
                .enabled = default_enabled2,\
            },\
        },\
    }\

void apply_active_profile();
void switch_active_profile(size_t current_profile);

void profile_writeback_task_create();
void profile_writeback_task_delete();
void profile_writeback_task();

void profile_update_equalizer_gains(float equalizer_gains[10]);
void profile_update_equalizer_enable(bool enabled);
void profile_update_mixer_gain(unsigned source_idx, float gain_db);
void profile_update_mixer_enable(unsigned source_idx, bool enabled);

uint8_t* serialize_profile(profile* input);
profile* deserialize_profile(uint8_t* data);
void fs_save_profiles(profile **profiles, size_t *current_profile, size_t num_profiles);
void fs_get_profiles(profile **profiles, size_t *current_profile, size_t num_profiles);
void fs_profiles_init(size_t num_profiles);

void fs_init();
void fs_deinit();

#endif // FS_H_