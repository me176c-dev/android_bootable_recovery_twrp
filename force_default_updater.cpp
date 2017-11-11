#include <unistd.h>
#include <string>
#include <android-base/strings.h>

#include "minzip/Zip.h"

#define METADATA_PATH  "META-INF/com/android/metadata"

#define DEFAULT_UPDATER_PATH  "/sbin/default-updater"

static bool read_metadata_from_package(ZipArchive* zip, std::string* meta_data) {
    const ZipEntry* meta_entry = mzFindZipEntry(zip, METADATA_PATH);
    if (meta_entry == nullptr) {
        printf("Failed to find " METADATA_PATH " in update package.\n");
        return false;
    }

    meta_data->resize(meta_entry->uncompLen, '\0');
    if (!mzReadZipEntry(zip, meta_entry, &(*meta_data)[0], meta_entry->uncompLen)) {
        printf("Failed to read metadata in update package.\n");
        return false;
    }
    return true;
}

bool link_default_updater(ZipArchive* zip, const char *updater_path) {
    std::string meta_data;
    if (!read_metadata_from_package(zip, &meta_data)) {
        return false;
    }

    std::vector<std::string> lines = android::base::Split(meta_data, "\n");
    for (const std::string& line : lines) {
        std::string str = android::base::Trim(line);
        if (android::base::StartsWith(str, "post-build")) {
            size_t pos = str.find('=');
            if (pos != std::string::npos) {
                std::string fingerprint = android::base::Trim(str.substr(pos+1));
                if (android::base::StartsWith(fingerprint, FORCE_DEFAULT_UPDATER_FINGERPRINT)) {
                    return symlink(DEFAULT_UPDATER_PATH, updater_path) == 0;
                }
            }
        }
    }

    return false;
}
