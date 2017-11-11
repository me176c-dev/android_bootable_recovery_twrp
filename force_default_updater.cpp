#include <unistd.h>
#include <string>
#include <android-base/strings.h>

#include "zipwrap.hpp"

#define METADATA_PATH  "META-INF/com/android/metadata"

#define DEFAULT_UPDATER_PATH  "/sbin/default-updater"

bool read_metadata_from_package(ZipWrap* zip, std::string* meta_data) {
    long size = zip->GetUncompressedSize(METADATA_PATH);
    if (size <= 0)
		return false;

    meta_data->resize(size, '\0');
    if (!zip->ExtractToBuffer(METADATA_PATH, reinterpret_cast<uint8_t*>(&(*meta_data)[0]))) {
        printf("Failed to read metadata in update package.\n");
        return false;
    }
    return true;
}

bool link_default_updater(ZipWrap *Zip, const char *updater_path) {
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
