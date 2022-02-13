#include "profile.h"
#include "pipeline.h"
#include "esp_log.h"

size_t g_current_profile = 0;
profile* g_profiles[MAX_NUM_PROFILES];
bool write_needed = false;

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
        if (write_needed) {
            fs_save_profiles(g_profiles, &g_current_profile, MAX_NUM_PROFILES);
            write_needed = false;
        }
        vTaskDelay(100);
    }
}

void apply_active_profile() {
    profile* curr = g_profiles[g_current_profile];
    pipeline_update_equalizer_gains(&g_audiyour_pipeline, curr->equalizer.gains);
    pipeline_update_equalizer_enable(&g_audiyour_pipeline, curr->equalizer.enabled);

    for (unsigned i = 0; i < curr->mixer.num_sources; ++i) {
        pipeline_update_mixer_gain(&g_audiyour_pipeline, i, curr->mixer.settings.gains[i]);
        pipeline_update_mixer_enable(&g_audiyour_pipeline, i, curr->mixer.settings.enabled[i]);
    }
}

void switch_active_profile(size_t current_profile) {
    g_current_profile = current_profile;
    apply_active_profile();
}

void profile_update_equalizer_gains(float equalizer_gains[10]) {
    memcpy(g_profiles[g_current_profile]->equalizer.gains, equalizer_gains, sizeof(g_profiles[g_current_profile]->equalizer.gains));
    pipeline_update_equalizer_gains(&g_audiyour_pipeline, equalizer_gains);
    write_needed = true;
}

void profile_update_equalizer_enable(bool enabled) {
    g_profiles[g_current_profile]->equalizer.enabled = enabled;
    pipeline_update_equalizer_enable(&g_audiyour_pipeline, enabled);
    write_needed = true;
}

void profile_update_mixer_gain(unsigned source_idx, float gain_db) {
    g_profiles[g_current_profile]->mixer.settings.gains[source_idx] = gain_db;
    pipeline_update_mixer_gain(&g_audiyour_pipeline, source_idx, gain_db);
    write_needed = true;
}

void profile_update_mixer_enable(unsigned source_idx, bool enabled) {
   g_profiles[g_current_profile]->mixer.settings.enabled[source_idx] = enabled;
    pipeline_update_mixer_enable(&g_audiyour_pipeline, source_idx, enabled);
    write_needed = true;
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

void fs_save_profiles(profile **profiles, size_t* current_profile, size_t num_profiles) {
    ESP_LOGI(TAG, "saving profiles");
    FILE *file = fopen(profile_filename, "w");

    profile default_p = DEFAULT_PROFILE();
    size_t data_size = profile_size(&default_p);

    fwrite(current_profile, sizeof(size_t), 1, file);

    for (unsigned i = 0; i < num_profiles; ++i) {
        uint8_t* serialized = serialize_profile(profiles[i]);
        fwrite(serialized, sizeof(uint8_t), data_size, file);
        free(serialized);
    }

    fclose(file);
}

void fs_get_profiles(profile **profiles, size_t *current_profile, size_t num_profiles) {
    ESP_LOGI(TAG, "getting profiles");
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
        fs_get_profiles(profiles, current_profile, num_profiles);
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

