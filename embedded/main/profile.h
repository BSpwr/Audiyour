#ifndef PROFILE_H_
#define PROFILE_H_

#include "esp_littlefs.h"

#include "equalizer2.h"
#include "mixer.h"

#define MAX_DEVICENAME_LEN 32
typedef struct device_name_t { char name[MAX_DEVICENAME_LEN + 1]; } device_name_t;
extern device_name_t g_device_name;

#define MAX_NUM_PROFILES 5
#define MAX_PROFILE_NAME_LEN 32

typedef struct profile {
    char name[MAX_PROFILE_NAME_LEN];
    equalizer2_profile equalizer;
    mixer_profile mixer;
} profile;

extern size_t g_profile_idx;
extern profile* g_profiles[MAX_NUM_PROFILES];
extern bool g_profiles_save_needed[MAX_NUM_PROFILES];
extern bool g_profiles_load_needed[MAX_NUM_PROFILES];
extern bool g_profile_idx_save_needed;

#define DEFAULT_DEVICENAME()\
    {\
        .name = "Audiyour\0",\
    }\

#define DEFAULT_PROFILE()\
    {\
        .name = "Profile",\
        .equalizer = {\
            .gains = {0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f}, \
            .enabled = false, \
        },\
        .mixer = {\
            .num_sources = mixer_NUM_SOURCES,\
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

void fs_save_profiles(profile **profiles, size_t current_profile, size_t num_profiles);
void fs_save_profile(profile **profiles, size_t selected_profile);
void fs_save_current_profile_index(size_t current_profile);

void fs_load_profiles(profile **profiles, size_t *current_profile, size_t num_profiles);
void fs_load_profile(profile **profiles, size_t selected_profile);

void fs_profiles_init(size_t num_profiles);

void fs_devicename_load(device_name_t* device_name);
void fs_devicename_save(device_name_t* device_name);
void fs_devicename_init();
void handle_device_name_change(device_name_t* device_name);

void fs_init();
void fs_deinit();

#endif // FS_H_