/* Read-only DVD title catalogue for Jellyfin. SPDX-License-Identifier: GPL-2.0-or-later */
#include <inttypes.h>
#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <dvdnav/dvdnav.h>

static void log_message(void *context, dvdnav_logger_level_t level,
                        const char *format, va_list arguments)
{
    vfprintf(stderr, format, arguments);
    fputc('\n', stderr);
}

int main(int argc, char **argv)
{
    dvdnav_t *nav = NULL;
    int32_t count;
    int emitted = 0;
    dvdnav_logger_cb logger = { .pf_log = log_message };

    if (argc != 2) {
        fprintf(stderr, "Usage: jellyfin-disc-info DVD_DIRECTORY_OR_ISO\n");
        return 2;
    }
    if (dvdnav_open2(&nav, NULL, &logger, argv[1]) != DVDNAV_STATUS_OK)
        return 1;
    if (dvdnav_get_number_of_titles(nav, &count) != DVDNAV_STATUS_OK) {
        fprintf(stderr, "%s\n", dvdnav_err_to_string(nav));
        dvdnav_close(nav);
        return 1;
    }

    printf("[");
    for (int title = 1; title <= count; title++) {
        uint64_t *chapters = NULL, duration = 0;
        uint32_t chapter_count = dvdnav_describe_title_chapters(nav, title, &chapters, &duration);
        free(chapters);
        if (!chapter_count || !duration)
            continue;
        printf("%s{\"Number\":%d,\"RunTimeTicks\":%" PRIu64 ",\"ChapterCount\":%u}",
               emitted++ ? "," : "", title, duration * 1000 / 9, chapter_count);
    }
    printf("]\n");
    dvdnav_close(nav);
    if (!emitted) {
        fprintf(stderr, "No navigable DVD titles found\n");
        return 1;
    }
    return 0;
}
