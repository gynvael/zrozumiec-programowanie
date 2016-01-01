#include <stdio.h>
#include <windows.h>

static const char *DRIVE_TYPES_AS_TEXT[] = {
  "(unknown)",  // DRIVE_UNKNOWN
  "(root path invalid)",  // DRIVE_NO_ROOT_DIR
  "removable",  // DRIVE_REMOVABLE
  "fixed (hard disk / flash drive)",  // DRIVE_FIXED
  "remote (network)",  // DRIVE_REMOTE
  "CD/DVD/BR-ROM (or similar)",  // DRIVE_CDROM
  "RAM disk"  // DRIVE_RAMDISK
};

int main(void) {
  DWORD present_drives = GetLogicalDrives();
  int i;

  for (i = 0; i < 32; i++) {
    if ((present_drives & (1 << i))) {
      char drive_path[] = {
        'A' + i, ':', '\\', '\0'
      };
      UINT drive_type = GetDriveType(drive_path);

      const char *drive_type_as_text = "(unknown)";
      if (drive_type <= 6) {
        drive_type_as_text = DRIVE_TYPES_AS_TEXT[drive_type];
      }

      printf("%s\t%s\n", drive_path, drive_type_as_text);
    }
  }

  return 0;
}

