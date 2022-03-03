#include "profile.h"
#include "pipeline.h"
#include "esp_log.h"

size_t g_profile_idx = 0;
profile* g_profiles[MAX_NUM_PROFILES];
bool g_profiles_save_needed[MAX_NUM_PROFILES];
bool g_profiles_load_needed[MAX_NUM_PROFILES];
bool g_profile_idx_save_needed = false;

static const esp_vfs_littlefs_conf_t littlefs_conf = {
    .base_path = "/littlefs",
    .partition_label = "littlefs",
    .format_if_mount_failed = true,
    .dont_mount = false,
};

static const char* profile_filename = "/littlefs/profiles.bin";

static const char* TAG = "PROFILE";

static TaskHandle_t profile_writeback_task_handle = NULL;

void profile_writeback_task_create() {
    if (profile_writeback_task_handle) {
        ESP_LOGI(TAG, "Profile writeback task already started");
        return;
    }

    memset(g_profiles_save_needed, 0, MAX_NUM_PROFILES * sizeof(bool));
    memset(g_profiles_load_needed, 0, MAX_NUM_PROFILES * sizeof(bool));

    /* Create the task, storing the handle. */
    BaseType_t ret = xTaskCreate(
                    (void (*)(void *))profile_writeback_task,       /* Function that implements the task. */
                    "profile_writeback",          /* Text name for the task. */
                    4096,      /* Stack size in words, not bytes. */
                    NULL,    /* Parameter passed into the task. */
                    tskIDLE_PRIORITY,/* Priority at which the task is created. */
                    &profile_writeback_task_handle);      /* Used to pass out the created task's handle. */

    if(ret == pdPASS) {
        ESP_LOGI(TAG, "Profile writeback task started successfully");
    }
}

void profile_writeback_task_delete() {
    vTaskDelete(profile_writeback_task_handle);
    profile_writeback_task_handle = NULL;
}

void profile_writeback_task() {    
    while (true) {
        for (size_t i = 0; i < MAX_NUM_PROFILES; ++i) {
            if (g_profiles_save_needed[i]) {
                fs_save_profile(g_profiles, i);
                g_profiles_save_needed[i] = false;
            }
        }
        for (size_t i = 0; i < MAX_NUM_PROFILES; ++i) {
            if (g_profiles_load_needed[i]) {
                fs_load_profile(g_profiles, i);
                g_profiles_load_needed[i] = false;
                apply_active_profile();
            }
        }
        if (g_profile_idx_save_needed) {
            fs_save_current_profile_index(g_profile_idx);
            g_profile_idx_save_needed = false;
        }
        vTaskDelay(10);
    }
}

void apply_active_profile() {
    profile* curr = g_profiles[g_profile_idx];
    pipeline_update_equalizer_gains(&g_audiyour_pipeline, curr->equalizer.gains);
    pipeline_update_equalizer_enable(&g_audiyour_pipeline, curr->equalizer.enabled);

    for (unsigned i = 0; i < curr->mixer.num_sources; ++i) {
        pipeline_update_mixer_gain(&g_audiyour_pipeline, i, curr->mixer.settings.gains[i]);
        pipeline_update_mixer_enable(&g_audiyour_pipeline, i, curr->mixer.settings.enabled[i]);
    }
}

void switch_active_profile(size_t current_profile) {
    g_profile_idx = current_profile;
    apply_active_profile();
    g_profile_idx_save_needed = true;
}

void profile_update_equalizer_gains(float equalizer_gains[10]) {
    memcpy(g_profiles[g_profile_idx]->equalizer.gains, equalizer_gains, sizeof(g_profiles[g_profile_idx]->equalizer.gains));
    pipeline_update_equalizer_gains(&g_audiyour_pipeline, equalizer_gains);
}

void profile_update_equalizer_enable(bool enabled) {
    g_profiles[g_profile_idx]->equalizer.enabled = enabled;
    pipeline_update_equalizer_enable(&g_audiyour_pipeline, enabled);
}

void profile_update_mixer_gain(unsigned source_idx, float gain_db) {
    ESP_LOGI(TAG, "hrggg: %d", (int)g_profiles[g_profile_idx]->mixer.settings.gains);
    g_profiles[g_profile_idx]->mixer.settings.gains[source_idx] = gain_db;
    pipeline_update_mixer_gain(&g_audiyour_pipeline, source_idx, gain_db);
}

void profile_update_mixer_enable(unsigned source_idx, bool enabled) {
   g_profiles[g_profile_idx]->mixer.settings.enabled[source_idx] = enabled;
    pipeline_update_mixer_enable(&g_audiyour_pipeline, source_idx, enabled);
}

inline size_t profile_size(profile* input) {
    // profile.name
    size_t data_size = MAX_PROFILE_NAME_LEN * sizeof(char);
    // profile.equalizer
    data_size += sizeof(equalizer2_profile);
    // profile.mixer
    data_size += sizeof(size_t) + input->mixer.num_sources * (sizeof(float) + sizeof(bool));

    return data_size;
}

uint8_t* serialize_profile(profile* input) {
    size_t data_size = profile_size(input);

    uint8_t *data = calloc(data_size, sizeof(uint8_t));
    size_t offset = 0;

    // profile.name
    memcpy(data, &input->name[0], MAX_PROFILE_NAME_LEN * sizeof(char));
    offset += MAX_PROFILE_NAME_LEN * sizeof(char);
    // profile.equalizer
    memcpy(data + offset, &input->equalizer, sizeof(equalizer2_profile));
    offset += sizeof(equalizer2_profile);
    // profile.mixer.num_sources
    memcpy(data + offset, &input->mixer.num_sources, sizeof(size_t));
    offset += sizeof(size_t);
    // profile.mixer.settings.gains
    memcpy(data + offset, input->mixer.settings.gains, input->mixer.num_sources * sizeof(float));
    offset += input->mixer.num_sources * sizeof(float);
    // profile.mixer.settings.enabled
    memcpy(data + offset, input->mixer.settings.enabled, input->mixer.num_sources * sizeof(bool));

    return data;
}

profile* deserialize_profile(uint8_t* data) {
    profile *output = calloc(1, sizeof(profile));
    size_t offset = 0;

    // profile.name
    memcpy(&output->name[0], data, MAX_PROFILE_NAME_LEN * sizeof(char));
    offset += MAX_PROFILE_NAME_LEN * sizeof(char);
    // profile.equalizer
    memcpy(&output->equalizer, data + offset, sizeof(equalizer2_profile));
    offset += sizeof(equalizer2_profile);
    // profile.mixer.num_sources
    memcpy(&output->mixer.num_sources, data + offset, sizeof(size_t));
    offset += sizeof(size_t);
    // profile.mixer.settings.gains
    output->mixer.settings.gains = calloc(output->mixer.num_sources, sizeof(float));
    memcpy(output->mixer.settings.gains, data + offset, output->mixer.num_sources * sizeof(float));
    offset += output->mixer.num_sources * sizeof(float);
    // profile.mixer.settings.enabled
    output->mixer.settings.enabled = calloc(output->mixer.num_sources, sizeof(bool));
    memcpy(output->mixer.settings.enabled, data + offset, output->mixer.num_sources * sizeof(bool));

    return output;
}

void fs_save_profiles(profile **profiles, size_t current_profile, size_t num_profiles) {
    ESP_LOGI(TAG, "saving all profiles");
    FILE *file = fopen(profile_filename, "w");

    profile default_p = DEFAULT_PROFILE();
    size_t data_size = profile_size(&default_p);

    fwrite(&current_profile, sizeof(size_t), 1, file);

    for (unsigned i = 0; i < num_profiles; ++i) {
        uint8_t* serialized = serialize_profile(profiles[i]);
        fwrite(serialized, sizeof(uint8_t), data_size, file);
        free(serialized);
    }

    fclose(file);
}

void fs_save_profile(profile **profiles, size_t selected_profile) {
    ESP_LOGI(TAG, "saving profile: %d", selected_profile);
    FILE *file = fopen(profile_filename, "r+");

    profile default_p = DEFAULT_PROFILE();
    size_t data_size = profile_size(&default_p);

    // seek to profile[selected_profile]
    fseek(file, sizeof(size_t) + data_size * selected_profile, SEEK_SET);

    uint8_t* serialized = serialize_profile(profiles[selected_profile]);
    fwrite(serialized, sizeof(uint8_t), data_size, file);
    free(serialized);

    fclose(file);
}

void fs_save_current_profile_index(size_t current_profile) {
    ESP_LOGI(TAG, "saving current profile index: %d", current_profile);
    FILE *file = fopen(profile_filename, "r+");

    profile default_p = DEFAULT_PROFILE();
    size_t data_size = profile_size(&default_p);

    fwrite(&current_profile, sizeof(size_t), 1, file);

    fclose(file);
}

// Load just the selected profile
void fs_load_profile(profile **profiles, size_t selected_profile) {
    ESP_LOGI(TAG, "getting profile: %d", selected_profile);
    FILE *file;

    profile default_p = DEFAULT_PROFILE();
    size_t data_size = profile_size(&default_p);
    
    if ((file = fopen(profile_filename, "r"))) {
        fseek(file, sizeof(size_t) + data_size * selected_profile, SEEK_SET);

        uint8_t* serialized = calloc(data_size, sizeof(uint8_t));
        fread(serialized, sizeof(uint8_t), data_size, file);
        profile* p = deserialize_profile(serialized);

        free(serialized);
        serialized = NULL;

        free(profiles[selected_profile]);
        profiles[selected_profile] = p;

        fclose(file);
    } else {
        ESP_LOGE(TAG, "file corrupt");
        // Initialize file
        fs_profiles_init(MAX_NUM_PROFILES);
    }
}

// Load all the profiles and also the current_profile index
void fs_load_profiles(profile **profiles, size_t *current_profile, size_t num_profiles) {
    ESP_LOGI(TAG, "getting all profiles");
    FILE *file;

    profile default_p = DEFAULT_PROFILE();
    size_t data_size = profile_size(&default_p);
    
    if ((file = fopen(profile_filename, "r"))) {
        fread(current_profile, sizeof(size_t), 1, file);

        for (unsigned i = 0; i < num_profiles; ++i) {
            uint8_t* serialized = calloc(data_size, sizeof(uint8_t));
            fread(serialized, sizeof(uint8_t), data_size, file);
            profile* p = deserialize_profile(serialized);

            free(serialized);
            serialized = NULL;

            profile* temp = profiles[i];
            profiles[i] = p;
            free(temp);
            temp = NULL;
        }

        fclose(file);
    } else {
        // Initialize file
        fs_profiles_init(num_profiles);
        fs_load_profiles(profiles, current_profile, num_profiles);
    }
}

void fs_profiles_init(size_t num_profiles) {
    ESP_LOGI(TAG, "initializing profiles");
    FILE *file = fopen(profile_filename, "w");

    profile default_p = DEFAULT_PROFILE();
    size_t data_size = profile_size(&default_p);

    uint8_t* serialized = serialize_profile(&default_p);

    size_t default_current_profile = 0;
    fwrite(&default_current_profile, sizeof(size_t), 1, file);

    for (unsigned i = 0; i < num_profiles; ++i) {
        fwrite(serialized, sizeof(uint8_t), data_size, file);
    }

    free(serialized);
    
    fclose(file);
}

void fs_init() {
    // Note: esp_vfs_littlefs_register is an all-in-one convenience function.
    esp_err_t ret = esp_vfs_littlefs_register(&littlefs_conf);

    if (ret != ESP_OK)
    {
            if (ret == ESP_FAIL)
            {
                    ESP_LOGE(TAG, "Failed to mount or format filesystem");
            }
            else if (ret == ESP_ERR_NOT_FOUND)
            {
                    ESP_LOGE(TAG, "Failed to find LittleFS partition");
            }
            else
            {
                    ESP_LOGE(TAG, "Failed to initialize LittleFS (%s)", esp_err_to_name(ret));
            }
            return;
    }
    ESP_LOGI(TAG, "LittleFS mounted");
}

void fs_deinit() {
    // All done, unmount partition and disable LittleFS
    esp_vfs_littlefs_unregister(littlefs_conf.partition_label);
    ESP_LOGI(TAG, "LittleFS unmounted");
}

