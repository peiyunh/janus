#include <stddef.h>
#include <string.h>

#include <pittpatt_raw_image_io.h>
#include <pittpatt_video_io.h>

#include "janus_io.h"

static janus_image janus_from_pittpatt(ppr_raw_image_type *ppr_image)
{
    if (!ppr_image)
        return NULL;

    if ((ppr_image->color_space != PPR_RAW_IMAGE_GRAY8) && (ppr_image->color_space != PPR_RAW_IMAGE_BGR24)) {
        ppr_raw_image_error_type error = ppr_raw_image_convert(ppr_image, PPR_RAW_IMAGE_BGR24);
        if (error != PPR_RAW_IMAGE_SUCCESS) return NULL;
    }

    janus_image image;
    janus_allocate_image(ppr_image->color_space == PPR_RAW_IMAGE_GRAY8 ? 1 : 3, ppr_image->width, ppr_image->height, &image);
    const janus_size elements_per_row = image->channels * image->width * sizeof(janus_data);
    for (int i=0; i<ppr_image->height; i++)
        memcpy(image->data + i*elements_per_row, ppr_image->data + i*ppr_image->bytes_per_line, elements_per_row);
    return image;
}

janus_error janus_read_image(const char *file_name, janus_image *image)
{
    ppr_raw_image_type ppr_image;
    ppr_raw_image_error_type error = ppr_raw_image_io_read(file_name, &ppr_image);
    if (error != PPR_RAW_IMAGE_SUCCESS) {
        *image = NULL;
        return JANUS_INVALID_IMAGE;
    }

    *image = janus_from_pittpatt(&ppr_image);
    ppr_raw_image_free(ppr_image);
    return JANUS_SUCCESS;
}

janus_error janus_open_video(const char *file_name, janus_video *video)
{
    ppr_video_io_type ppr_video;
    ppr_video_io_error_type error = ppr_video_io_open(&ppr_video, file_name);
    if (error != PPR_VIDEO_IO_SUCCESS) {
        *video = NULL;
        return JANUS_INVALID_VIDEO;
    }
    *video = (janus_video)ppr_video;
    return JANUS_SUCCESS;
}

janus_error janus_read_frame(janus_video video, janus_image *image)
{
    ppr_raw_image_type ppr_frame;
    ppr_video_io_error_type error = ppr_video_io_get_frame((ppr_video_io_type)video, &ppr_frame);
    error = error || ppr_video_io_step_forward((ppr_video_io_type)video);
    if (error != PPR_VIDEO_IO_SUCCESS) {
        *image = NULL;
        return JANUS_INVALID_VIDEO;
    }

    *image = janus_from_pittpatt(&ppr_frame);
    ppr_raw_image_free(ppr_frame);
    return JANUS_SUCCESS;
}

void janus_close_video(janus_video video)
{
    ppr_video_io_close((ppr_video_io_type)video);
}
