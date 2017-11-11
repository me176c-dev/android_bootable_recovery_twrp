#include <unistd.h>
#include <string>
#include <android-base/strings.h>

#include "zipwrap.hpp"

#define METADATA_PATH  "META-INF/com/android/metadata"

#define CUSTOM_UPDATER_PATH  "/sbin/" FORCE_CUSTOM_UPDATER

static bool read_metadata_from_package(ZipWrap* Zip, std::string* meta_data) {
    long size = Zip->GetUncompressedSize(METADATA_PATH);
    if (size <= 0)
		return false;

    meta_data->resize(size, '\0');
    if (!Zip->ExtractToBuffer(METADATA_PATH, reinterpret_cast<uint8_t*>(&(*meta_data)[0]))) {
        printf("Failed to read metadata in update package.\n");
        return false;
    }
    return true;
}

bool link_custom_updater(ZipWrap *Zip, const char *updater_path) {
    std::string meta_data;
    if (!read_metadata_from_package(Zip, &meta_data)) {
        return false;
    }

    std::vector<std::string> lines = android::base::Split(meta_data, "\n");
    for (const std::string& line : lines) {
        std::string str = android::base::Trim(line);
        if (android::base::StartsWith(str, "post-build")) {
            size_t pos = str.find('=');
            if (pos != std::string::npos) {
                std::string fingerprint = android::base::Trim(str.substr(pos+1));
                if (android::base::StartsWith(fingerprint, FORCE_CUSTOM_UPDATER_FINGERPRINT)) {
                    return symlink(CUSTOM_UPDATER_PATH, updater_path) == 0;
                }
            }
        }
    }

    return false;
}
