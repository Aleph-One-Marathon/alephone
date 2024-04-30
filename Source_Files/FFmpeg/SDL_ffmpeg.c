/*******************************************************************************
*                                                                              *
*   SDL_ffmpeg is a library for basic multimedia functionality.                *
*   SDL_ffmpeg is based on ffmpeg.                                             *
*                                                                              *
*   Copyright (C) 2007  Arjan Houben                                           *
*                                                                              *
*   SDL_ffmpeg is free software: you can redistribute it and/or modify         *
*   it under the terms of the GNU Lesser General Public License as published   *
*   by the Free Software Foundation, either version 3 of the License, or any   *
*   later version.                                                             *
*                                                                              *
*   This program is distributed in the hope that it will be useful,            *
*   but WITHOUT ANY WARRANTY; without even the implied warranty of             *
*   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the               *
*   GNU Lesser General Public License for more details.                        *
*                                                                              *
*   You should have received a copy of the GNU Lesser General Public License   *
*   along with this program.  If not, see <http://www.gnu.org/licenses/>.      *
*                                                                              *
*******************************************************************************/

/**
    @mainpage
    @version 1.3.2
    @author Arjan Houben

    SDL_ffmpeg is designed with ease of use in mind.
    Even the beginning programmer should be able to use this library
    so he or she can use multimedia in his/her program.
**/
#ifdef HAVE_CONFIG_H // for HAVE_FFMPEG
#include "config.h"
#endif

#ifdef HAVE_FFMPEG

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL2/SDL.h>
#include <SDL2/SDL_thread.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include "libavcodec/avcodec.h"
#include "libavformat/avformat.h"
#include "libavutil/mathematics.h"
#include "libavutil/imgutils.h"
#include "libswscale/swscale.h"
#include "libswresample/swresample.h"
#include "libavutil/opt.h"
#ifdef __cplusplus
}
#endif

// FFmpeg compatibility
#ifndef AVCODEC_MAX_AUDIO_FRAME_SIZE
#define AVCODEC_MAX_AUDIO_FRAME_SIZE 192000
#endif

#include "SDL_ffmpeg.h"

#ifdef MSVC
#define snprintf( buf, count, format, ... )  _snprintf_s( buf, 512, count, format, __VA_ARGS__ )
#ifndef INT64_C
#define INT64_C(i) i
#endif
#endif


/**
\cond
*/

/**
 *  Provide a fast way to get the correct context.
 *  \returns The context matching the input values.
 */
struct SwsContext* getContext( SDL_ffmpegConversionContext **context, int inWidth, int inHeight, enum AVPixelFormat inFormat, int outWidth, int outHeight, enum AVPixelFormat outFormat )
{
    SDL_ffmpegConversionContext *ctx = *context;

    if ( ctx )
    {
        /* check for a matching context */
        while ( ctx )
        {
            if ( ctx->inWidth == inWidth &&
                    ctx->inHeight == inHeight &&
                    ctx->inFormat == inFormat &&
                    ctx->outWidth == outWidth &&
                    ctx->outHeight == outHeight &&
                    ctx->outFormat == outFormat )
            {
                return ctx->context;
            }
			
            ctx = ctx->next;
        }

        ctx = *context;

        /* find the last context */
        while ( ctx && ctx->next ) ctx = ctx->next;

        /* allocate a new context */
        ctx->next = ( struct SDL_ffmpegConversionContext* ) malloc( sizeof( SDL_ffmpegConversionContext ) );
		ctx = ctx->next;
    }
    else
    {
        ctx = *context = ( struct SDL_ffmpegConversionContext* ) malloc( sizeof( SDL_ffmpegConversionContext ) );
        // TODO handle the case where ctx is still null due to malloc failure
    }

    /* fill context with correct information */
    ctx->context = sws_getContext( inWidth, inHeight, inFormat,
                                   outWidth, outHeight, outFormat,
                                   SWS_BILINEAR,
                                   0,
                                   0,
                                   0 );

    ctx->inWidth = inWidth;
    ctx->inHeight = inHeight;
    ctx->inFormat = inFormat;
    ctx->outWidth = outWidth;
    ctx->outHeight = outHeight;
    ctx->outFormat = outFormat;
    ctx->next = 0;

    return ctx->context;
}

/* error handling */
char SDL_ffmpegErrorMessage[ 512 ];

void SDL_ffmpegSetError( const char *error );

/* packet handling */
int SDL_ffmpegGetPacket( SDL_ffmpegFile* );

SDL_ffmpegPacket* SDL_ffmpegGetAudioPacket( SDL_ffmpegFile* );

SDL_ffmpegPacket* SDL_ffmpegGetVideoPacket( SDL_ffmpegFile* );

/* frame handling */
int SDL_ffmpegDecodeAudioFrame( SDL_ffmpegFile*, AVPacket*, SDL_ffmpegAudioFrame* );

int SDL_ffmpegDecodeVideoFrame( SDL_ffmpegFile*, AVPacket*, SDL_ffmpegVideoFrame* );

SDL_ffmpegFile* SDL_ffmpegCreateFile()
{
    /* create SDL_ffmpegFile pointer */
    SDL_ffmpegFile *file = ( SDL_ffmpegFile* )malloc( sizeof( SDL_ffmpegFile ) );
    if ( !file )
    {
        SDL_ffmpegSetError( "could not allocate SDL_ffmpegFile" );
        return 0;
    }

    memset( file, 0, sizeof( SDL_ffmpegFile ) );

    file->streamMutex = SDL_CreateMutex();

    return file;
}

/**
\endcond
*/


/** \brief  Use this to free an SDL_ffmpegFile.

            This function stops the decoding thread if needed
            and flushes the buffers before releasing the memory.
\param      file SDL_ffmpegFile which needs to be removed
*/
void SDL_ffmpegFree( SDL_ffmpegFile *file )
{
    if ( !file ) return;

    SDL_ffmpegFlush( file );

    /* only write trailer when handling output streams */
    if ( file->type == SDL_ffmpegOutputStream )
    {
        av_write_trailer( file->_ffmpeg );
    }

    SDL_ffmpegStream *s = file->vs;
    while ( s )
    {
        SDL_ffmpegStream *old = s;

        s = s->next;

        SDL_DestroyMutex( old->mutex );

        while ( old->buffer )
        {
            SDL_ffmpegPacket *pack = old->buffer;

            old->buffer = old->buffer->next;

            av_packet_free( &pack->data );

            free( pack );
        }

        while ( old->conversionContext )
        {
            SDL_ffmpegConversionContext *ctx = old->conversionContext;

            old->conversionContext = old->conversionContext->next;

            sws_freeContext( ctx->context );

            free( ctx );
        }

        av_frame_free( &old->encodeFrame );
        av_frame_free( &old->decodeFrame );

        if ( old->_ctx) avcodec_free_context( &old->_ctx );

        free( old );
    }

    s = file->as;
    while ( s )
    {
        SDL_ffmpegStream *old = s;

        s = s->next;

        SDL_DestroyMutex( old->mutex );

        while ( old->buffer )
        {
            SDL_ffmpegPacket *pack = old->buffer;

            old->buffer = old->buffer->next;

            av_packet_free( &pack->data );

            free( pack );
        }

        av_free( old->sampleBuffer );
        av_frame_free( &old->encodeFrame );
        av_frame_free( &old->decodeFrame );

        if ( old->_ctx) avcodec_free_context( &old->_ctx );

        if (old->swr_context) swr_free(&old->swr_context);

        free( old );
    }

    if ( file->_ffmpeg )
    {
        if ( file->type == SDL_ffmpegInputStream )
        {
            avformat_close_input( &file->_ffmpeg );
        }
        else if ( file->type == SDL_ffmpegOutputStream )
        {
            avio_close( file->_ffmpeg->pb );

            av_free( file->_ffmpeg );
        }
    }

    SDL_DestroyMutex( file->streamMutex );

    free( file );
}


/** \brief  Use this to free an SDL_ffmpegAudioFrame.

            This releases all buffers which where allocated in SDL_ffmpegCreateAudioFrame
\param      frame SDL_ffmpegAudioFrame which needs to be deleted
*/
void SDL_ffmpegFreeAudioFrame( SDL_ffmpegAudioFrame* frame )
{
    av_free( frame->buffer );
    if (frame->conversionBuffer)
    {
        av_freep(&frame->conversionBuffer[0]);
        av_freep(&frame->conversionBuffer);
    }
    free( frame );
}


/** \brief  Use this to free an SDL_ffmpegVideoFrame.

            This releases all buffers which where allocated in SDL_ffmpegCreateVideoFrame
\param      frame SDL_ffmpegVideoFrame which needs to be deleted
*/
void SDL_ffmpegFreeVideoFrame( SDL_ffmpegVideoFrame* frame )
{
    if ( !frame ) return;

    if ( frame->surface ) SDL_FreeSurface( frame->surface );

//    if ( frame->overlay ) SDL_FreeYUVOverlay( frame->overlay );

    free( frame );
}


/** \brief  Use this to open the multimedia file of your choice.

            This function is used to open a multimedia file.
            When the file could be opened, but no decodable streams where detected
            this function still returns a pointer to a valid SDL_ffmpegFile.
\param      filename string containing the location of the file
\returns    a pointer to a SDL_ffmpegFile structure, or NULL if a file could not be opened
*/
SDL_ffmpegFile* SDL_ffmpegOpen( const char* filename )
{
    /* open new ffmpegFile */
    SDL_ffmpegFile *file = SDL_ffmpegCreateFile();
    if ( !file ) return 0;

    /* information about format is stored in file->_ffmpeg */
    file->type = SDL_ffmpegInputStream;

    /* open the file */
    if ( avformat_open_input(&file->_ffmpeg, filename, NULL, NULL) != 0 )
    {
        char c[512];
        snprintf( c, 512, "could not open \"%s\"", filename );
        SDL_ffmpegSetError( c );
        free( file );
        return 0;
    }

    /* retrieve format information */
    if ( avformat_find_stream_info( file->_ffmpeg, NULL ) < 0 )
    {
        char c[512];
        snprintf( c, 512, "could not retrieve file info for \"%s\"", filename );
        SDL_ffmpegSetError( c );
        free( file );
        return 0;
    }

    /* iterate through all the streams and store audio/video streams */
    for ( uint32_t i = 0; i < file->_ffmpeg->nb_streams; i++ )
    {
        /* disable all streams by default */
        file->_ffmpeg->streams[i]->discard = AVDISCARD_ALL;

        if ( file->_ffmpeg->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_VIDEO )
        {
            /* if this is a packet of the correct type we create a new stream */
            SDL_ffmpegStream* stream = ( SDL_ffmpegStream* )malloc( sizeof( SDL_ffmpegStream ) );

            if ( stream )
            {
                /* we set our stream to zero */
                memset( stream, 0, sizeof( SDL_ffmpegStream ) );

                /* save unique streamid */
                stream->id = i;

                /* _ffmpeg holds data about streamcodec */
                stream->_ffmpeg = file->_ffmpeg->streams[i];

                /* get the correct decoder for this stream */
                AVCodec *codec = avcodec_find_decoder( stream->_ffmpeg->codecpar->codec_id );

                if ( !codec )
                {
                    free( stream );
                    SDL_ffmpegSetError( "could not find video codec" );
                    continue;
                }
                stream->_ctx = avcodec_alloc_context3(codec);
                if (!stream->_ctx || avcodec_parameters_to_context(stream->_ctx, stream->_ffmpeg->codecpar) < 0 
                    || avcodec_open2(stream->_ctx, NULL, NULL ) < 0 || avcodec_parameters_from_context(stream->_ffmpeg->codecpar, stream->_ctx) < 0)
                {
                    free( stream );
                    SDL_ffmpegSetError( "could not open video codec" );
                }
                else
                {
                    stream->mutex = SDL_CreateMutex();

                    stream->decodeFrame = av_frame_alloc();

                    SDL_ffmpegStream **s = &file->vs;
                    while ( *s )
                    {
                        *s = ( *s )->next;
                    }

                    *s = stream;

                    file->videoStreams++;
                }
            }
        }
        else if ( file->_ffmpeg->streams[i]->codecpar->codec_type == AVMEDIA_TYPE_AUDIO )
        {
            /* if this is a packet of the correct type we create a new stream */
            SDL_ffmpegStream* stream = ( SDL_ffmpegStream* )malloc( sizeof( SDL_ffmpegStream ) );

            if ( stream )
            {
                /* we set our stream to zero */
                memset( stream, 0, sizeof( SDL_ffmpegStream ) );

                /* save unique streamid */
                stream->id = i;

                /* _ffmpeg holds data about streamcodec */
                stream->_ffmpeg = file->_ffmpeg->streams[i];

                /* get the correct decoder for this stream */
                AVCodec *codec = avcodec_find_decoder( file->_ffmpeg->streams[i]->codecpar->codec_id );

                if ( !codec )
                {
                    free( stream );
                    SDL_ffmpegSetError( "could not find audio codec" );
                    continue;
                }
                stream->_ctx = avcodec_alloc_context3(codec);
                if (!stream->_ctx || avcodec_parameters_to_context(stream->_ctx, stream->_ffmpeg->codecpar) < 0 
                    || avcodec_open2(stream->_ctx, NULL, NULL) < 0 || avcodec_parameters_from_context(stream->_ffmpeg->codecpar, stream->_ctx) < 0)
                {
                    free( stream );
                    SDL_ffmpegSetError( "could not open audio codec" );
                }
                else
                {
                    int channel_layout = stream->_ffmpeg->codecpar->channel_layout ? stream->_ffmpeg->codecpar->channel_layout : 
                        (stream->_ffmpeg->codecpar->channels == 2 ? AV_CH_LAYOUT_STEREO : AV_CH_LAYOUT_MONO);

                    stream->swr_context = swr_alloc_set_opts(stream->swr_context, channel_layout, AV_SAMPLE_FMT_FLT,
                        stream->_ffmpeg->codecpar->sample_rate, channel_layout,
                        stream->_ffmpeg->codecpar->format, stream->_ffmpeg->codecpar->sample_rate,
                        0, NULL);

                    if (!stream->swr_context || swr_init(stream->swr_context) < 0) {
                        free(stream);
                        SDL_ffmpegSetError("could not initialize resampler");
                        continue;
                    }

                    stream->mutex = SDL_CreateMutex();
                    stream->encodeFrame = av_frame_alloc();
                    stream->decodeFrame = av_frame_alloc();
                    stream->sampleBuffer = ( int8_t* )av_malloc( AVCODEC_MAX_AUDIO_FRAME_SIZE * sizeof( int16_t ) );
                    stream->sampleBufferSize = 0;
                    stream->sampleBufferOffset = 0;
                    stream->sampleBufferTime = AV_NOPTS_VALUE;

                    SDL_ffmpegStream **s = &file->as;
                    while ( *s )
                    {
                        *s = ( *s )->next;
                    }

                    *s = stream;

                    file->audioStreams++;
                }
            }
        }
    }

    return file;
}

/** \brief  Use this to create the multimedia file of your choice.

            This function is used to create a multimedia file.

\param      filename string containing the location to which the data will be written
\returns    a pointer to a SDL_ffmpegFile structure, or NULL if a file could not be opened
*/
SDL_ffmpegFile* SDL_ffmpegCreate( const char* filename )
{
    SDL_ffmpegFile *file = SDL_ffmpegCreateFile();

    file->_ffmpeg = avformat_alloc_context();

    file->_ffmpeg->oformat = av_guess_format( "webm", 0, 0 );
    file->_ffmpeg->url = filename;

    /* open the output file, if needed */
    if (avio_open(&file->_ffmpeg->pb, file->_ffmpeg->url, AVIO_FLAG_WRITE ) < 0 )
    {
        char c[512];
        snprintf( c, 512, "could not open \"%s\"", filename );
        SDL_ffmpegSetError( c );
        SDL_ffmpegFree( file );
        return 0;
    }

    file->type = SDL_ffmpegOutputStream;

    return file;
}


/** \brief  Use this to add a SDL_ffmpegVideoFrame to file

            By adding frames to file, a video stream is build. If an audio stream
            is present, syncing of both streams needs to be done by user.
\param      file SDL_ffmpegFile to which a frame needs to be added.
\param      frame SDL_Surface which will be added to the stream.
\param      frame number
\param      set to 1 to warn this is the last frame
\returns    0 if frame was added, non-zero if an error occured.
*/
int SDL_ffmpegAddVideoFrame( SDL_ffmpegFile *file, SDL_Surface *sdlFrame, int32_t frameNumber, int32_t lastFrame )
{
    /* when accesing audio/video stream, streamMutex should be locked */
    SDL_LockMutex( file->streamMutex );

    if ( !file->videoStream || (!lastFrame && (!sdlFrame || !sdlFrame->format)))
    {
        SDL_UnlockMutex( file->streamMutex );
        return -1;
    }

    AVFrame* frame = NULL;
    if (!lastFrame)
    {
        frame = file->videoStream->encodeFrame;

        int pitch[] =
        {
            sdlFrame->pitch,
            0
        };

        const uint8_t* const data[] =
        {
            (uint8_t*)sdlFrame->pixels,
            0
        };

        switch (sdlFrame->format->BitsPerPixel)
        {
            case 24:
                sws_scale(getContext(&file->videoStream->conversionContext,
                    sdlFrame->w, sdlFrame->h, AV_PIX_FMT_RGB24,
                    file->videoStream->_ffmpeg->codecpar->width,
                    file->videoStream->_ffmpeg->codecpar->height,
                    file->videoStream->_ffmpeg->codecpar->format),
                    data,
                    pitch,
                    0,
                    sdlFrame->h,
                    frame->data,
                    frame->linesize);
                break;
            case 32:
                sws_scale(getContext(&file->videoStream->conversionContext,
                    sdlFrame->w, sdlFrame->h, AV_PIX_FMT_RGB32,
                    file->videoStream->_ffmpeg->codecpar->width,
                    file->videoStream->_ffmpeg->codecpar->height,
                    file->videoStream->_ffmpeg->codecpar->format),
                    data,
                    pitch,
                    0,
                    sdlFrame->h,
                    frame->data,
                    frame->linesize);
                break;
            default:
                break;
        }

        //Needed since ffmpeg version 4.4
        frame->format = file->videoStream->_ctx->pix_fmt;
        frame->width = file->videoStream->_ctx->width;
        frame->height = file->videoStream->_ctx->height;

        frame->pts = frameNumber;
    }

    int32_t done = 0;
    AVPacket* pkt = av_packet_alloc();
    while (!done)
    {
        // add video
        pkt->data = file->videoStream->encodeFrameBuffer;
        pkt->size = file->videoStream->encodeFrameBufferSize;

        int vsize = avcodec_send_frame(file->videoStream->_ctx, frame);
        int got_packet = avcodec_receive_packet(file->videoStream->_ctx, pkt);

        if (vsize == 0 && got_packet == 0)
        {
            if (pkt->pts != AV_NOPTS_VALUE && pkt->pts < pkt->dts) pkt->pts = pkt->dts;
            if (pkt->pts != AV_NOPTS_VALUE) pkt->pts = av_rescale_q(pkt->pts, file->videoStream->_ctx->time_base, file->videoStream->_ffmpeg->time_base);
            if (pkt->dts != AV_NOPTS_VALUE) pkt->dts = av_rescale_q(pkt->dts, file->videoStream->_ctx->time_base, file->videoStream->_ffmpeg->time_base);
            pkt->duration = av_rescale_q(pkt->duration, file->videoStream->_ctx->time_base, file->videoStream->_ffmpeg->time_base);
            pkt->stream_index = file->videoStream->_ffmpeg->index;
            av_interleaved_write_frame(file->_ffmpeg, pkt);
            av_packet_unref(pkt);
        }

        if (!lastFrame || vsize < 0 || got_packet < 0) done = 1;
    }
    av_packet_free(&pkt);

    SDL_UnlockMutex( file->streamMutex );

    return 0;
}


/** \brief  Use this to add a SDL_ffmpegAudioFrame to file

            By adding frames to file, an audio stream is build. If a video stream
            is present, syncing of both streams needs to be done by user.
\param      file SDL_ffmpegFile to which a frame needs to be added.
\param      frame SDL_ffmpegAudioFrame which will be added to the stream.
\returns    0 if frame was added, non-zero if an error occured.
*/
int SDL_ffmpegAddAudioFrame( SDL_ffmpegFile *file, SDL_ffmpegAudioFrame *frame, size_t* frameCounter, int32_t lastFrame )
{
    /* when accesing audio/video stream, streamMutex should be locked */
    SDL_LockMutex( file->streamMutex );

    if ( !file  || !file->audioStream || (!frame && !lastFrame))
    {
        SDL_UnlockMutex( file->streamMutex );
        return -1;
    }

    AVCodecContext* acodec = file->audioStream->_ctx;

    // convert
    int32_t write_bps = av_get_bytes_per_sample(acodec->sample_fmt);
    int32_t read_samples = frame->size / (av_get_bytes_per_sample(file->audioStream->audioFormat) * acodec->channels);
    int32_t write_samples = read_samples;
    if (read_samples < acodec->frame_size)
    {
        // shrink or pad audio frame
        if (acodec->codec->capabilities & AV_CODEC_CAP_SMALL_LAST_FRAME)
            acodec->frame_size = write_samples;
        else
            write_samples = acodec->frame_size;
    }

    write_samples = swr_convert(file->audioStream->swr_context, frame->conversionBuffer, write_samples, (const uint8_t**)&frame->buffer, read_samples);

    AVFrame* audio_frame = file->audioStream->encodeFrame;
    av_frame_unref(audio_frame);

    //Needed since ffmpeg 4.4
    audio_frame->channels = acodec->channels;
    audio_frame->format = acodec->sample_fmt;
    audio_frame->channel_layout = acodec->channel_layout;
    audio_frame->sample_rate = acodec->sample_rate;
    audio_frame->nb_samples = write_samples;

    AVRational avSampleRate = { 1, acodec->sample_rate };
    audio_frame->pts = av_rescale_q(*frameCounter, avSampleRate, acodec->time_base);

    *frameCounter += write_samples;
    int asize = avcodec_fill_audio_frame(audio_frame, acodec->channels,
        acodec->sample_fmt,
        frame->conversionBuffer[0],
        write_samples * write_bps * acodec->channels, 1);

    if (asize >= 0)
    {
        AVPacket* pkt = av_packet_alloc();
        int vsize = avcodec_send_frame(acodec, audio_frame);
        if (0 == vsize)
        {
            while (avcodec_receive_packet(acodec, pkt) == 0) {
                if (pkt->pts != AV_NOPTS_VALUE && pkt->pts < pkt->dts) pkt->pts = pkt->dts;
                if (pkt->pts != AV_NOPTS_VALUE) pkt->pts = av_rescale_q(pkt->pts, acodec->time_base, file->audioStream->_ffmpeg->time_base);
                if (pkt->dts != AV_NOPTS_VALUE) pkt->dts = av_rescale_q(pkt->dts, acodec->time_base, file->audioStream->_ffmpeg->time_base);
                pkt->duration = av_rescale_q(pkt->duration, acodec->time_base, file->audioStream->_ffmpeg->time_base);
                pkt->stream_index = file->audioStream->_ffmpeg->index;
                file->audioStream->lastTimeStamp = pkt->pts + pkt->duration;
                av_interleaved_write_frame(file->_ffmpeg, pkt);
                av_packet_unref(pkt);
            }
        }
        av_packet_free(&pkt);
    }
    if (lastFrame)
    {
        int32_t done = 0;
        AVPacket* pkt = av_packet_alloc();
        int flush = avcodec_send_frame(acodec, NULL);
        while (flush == 0 && !done)
        {
            int got_pkt = avcodec_receive_packet(acodec, pkt);
            if (got_pkt == 0)
            {
                if (pkt->pts != AV_NOPTS_VALUE && pkt->pts < pkt->dts) pkt->pts = pkt->dts;
                if (pkt->pts != AV_NOPTS_VALUE) pkt->pts = av_rescale_q(pkt->pts, acodec->time_base, file->audioStream->_ffmpeg->time_base);
                if (pkt->dts != AV_NOPTS_VALUE) pkt->dts = av_rescale_q(pkt->dts, acodec->time_base, file->audioStream->_ffmpeg->time_base);
                pkt->duration = av_rescale_q(pkt->duration, acodec->time_base, file->audioStream->_ffmpeg->time_base);
                pkt->stream_index = file->audioStream->_ffmpeg->index;
                av_interleaved_write_frame(file->_ffmpeg, pkt);
                av_packet_unref(pkt);
            }
            else
            {
                done = 1;
            }
        }
        av_packet_free(&pkt);
    }

    file->audioStream->frameCount++;

    SDL_UnlockMutex( file->streamMutex );

    return 0;
}

/** \brief  Use this to create a SDL_ffmpegAudioFrame

            With this frame, you can receive audio data from the stream using
            SDL_ffmpegGetAudioFrame.
\param      file SDL_ffmpegFile for which a frame needs to be created
\param      bytes When current active audio stream is an input stream, this holds
                  the size of the buffer which will be allocated. In case of an
                  output stream, this value is ignored.
\returns    Pointer to SDL_ffmpegAudioFrame, or NULL if no frame could be created
*/
SDL_ffmpegAudioFrame* SDL_ffmpegCreateAudioFrame( SDL_ffmpegFile *file, uint32_t bytes )
{
	if (!file) {
		return 0;
	}
    /* when accesing audio/video stream, streamMutex should be locked */
    SDL_LockMutex( file->streamMutex );

    if (!file->audioStream || ( !bytes && file->type == SDL_ffmpegInputStream ) )
    {
        SDL_UnlockMutex( file->streamMutex );
        return 0;
    }

    /* allocate new frame */
    SDL_ffmpegAudioFrame *frame = ( SDL_ffmpegAudioFrame* )malloc( sizeof( SDL_ffmpegAudioFrame ) );
    memset( frame, 0, sizeof( SDL_ffmpegAudioFrame ) );

    if ( file->type == SDL_ffmpegOutputStream )
    {
        bytes = file->audioStream->encodeAudioInputSize * av_get_bytes_per_sample(file->audioStream->audioFormat) * file->audioStream->_ctx->channels;

        // allocate conversion buffer only when output, input does it differently
        if (av_samples_alloc_array_and_samples(&frame->conversionBuffer, NULL, file->audioStream->_ctx->channels, file->audioStream->encodeAudioInputSize, file->audioStream->_ctx->sample_fmt, 0) < 0)
        {
            return 0;
        }
    }

    SDL_UnlockMutex( file->streamMutex );

    /* set capacity of new frame */
    frame->capacity = bytes;

    /* allocate buffer */
    frame->buffer = ( uint8_t* )av_malloc( bytes );

    /* initialize a non-valid timestamp */
    frame->pts = AV_NOPTS_VALUE;

    return frame;
}


/** \brief  Use this to create a SDL_ffmpegVideoFrame

            In order to receive video data, either SDL_ffmpegVideoFrame.surface or
            SDL_ffmpegVideoFrame.overlay need to be set by user.
\returns    Pointer to SDL_ffmpegVideoFrame, or NULL if no frame could be created
*/
SDL_ffmpegVideoFrame* SDL_ffmpegCreateVideoFrame()
{
    SDL_ffmpegVideoFrame *frame = ( SDL_ffmpegVideoFrame* )malloc( sizeof( SDL_ffmpegVideoFrame ) );
    // TODO return failure if frame could not be allocated, unsure how that should look
    memset( frame, 0, sizeof( SDL_ffmpegVideoFrame ) );

    return frame;
}


/** \brief  Use this to get new video data from file.

            Using this function, you can retreive video data from file. This data
            gets written to frame.
\param      file SDL_ffmpegFile from which the data is required
\param      frame SDL_ffmpegVideoFrame to which the data will be written
\returns    non-zero when a frame was retreived, zero otherwise
*/
int SDL_ffmpegGetVideoFrame( SDL_ffmpegFile* file, SDL_ffmpegVideoFrame *frame )
{
	if (!file) {
		return 0;
	}
    /* when accesing audio/video stream, streamMutex should be locked */
    SDL_LockMutex( file->streamMutex );

    if ( !frame || !file || !file->videoStream )
    {
        SDL_UnlockMutex( file->streamMutex );
        return 0;
    }

    SDL_LockMutex( file->videoStream->mutex );

    /* assume current frame is empty */
    frame->ready = 0;
    frame->last = 0;

    /* get new packet */
    SDL_ffmpegPacket *pack = SDL_ffmpegGetVideoPacket( file );

    while ( !pack && !frame->last )
    {
        pack = SDL_ffmpegGetVideoPacket( file );

        frame->last = SDL_ffmpegGetPacket( file );
    }

    while ( pack && !frame->ready )
    {
        /* when a frame is received, frame->ready will be set */
        SDL_ffmpegDecodeVideoFrame( file, pack->data, frame );

        /* destroy used packet */
        av_packet_free( &pack->data );

        free( pack );

        pack = SDL_ffmpegGetVideoPacket( file );

        while ( !pack && !frame->last )
        {
            pack = SDL_ffmpegGetVideoPacket( file );

            frame->last = SDL_ffmpegGetPacket( file );
        }
    }

    /* pack retreived, but was not used, push it back in the buffer */
    if ( pack )
    {
        /* take current buffer as next pointer */
        pack->next = file->videoStream->buffer;

        /* store pack as current buffer */
        file->videoStream->buffer = pack;
    }
    else if ( !frame->ready && frame->last )
    {
        /* check if there is still a frame in the buffer */
        SDL_ffmpegDecodeVideoFrame( file, 0, frame );
    }

    SDL_UnlockMutex( file->videoStream->mutex );

    SDL_UnlockMutex( file->streamMutex );

    return frame->ready;
}


/** \brief  Get the desired audio stream from file.

            This returns a pointer to the requested stream. With this stream pointer you can
            get information about the stream, like language, samplerate, size etc.
            Based on this information you can choose the stream you want to use.
\param      file SDL_ffmpegFile from which the information is required
\param      audioID is the stream you whish to select.
\returns    Pointer to SDL_ffmpegStream, or NULL if selected stream could not be found
*/
SDL_ffmpegStream* SDL_ffmpegGetAudioStream( SDL_ffmpegFile *file, uint32_t audioID )
{
    /* check if we have any audiostreams */
    if ( !file || !file->audioStreams ) return 0;

    SDL_ffmpegStream *s = file->as;

    /* return audiostream linked to audioID */
    for ( uint32_t i = 0; i < audioID && s; i++ ) s = s->next;

    return s;
}


/** \brief  Select an audio stream from file.

            Use this function to select an audio stream for decoding.
            Using SDL_ffmpegGetAudioStream you can get information about the streams.
            Based on that you can chose the stream you want.
\param      file SDL_ffmpegFile on which an action is required
\param      audioID is the stream you whish to select. negative values de-select any audio stream.
\returns    -1 on error, otherwise 0
*/
int SDL_ffmpegSelectAudioStream( SDL_ffmpegFile* file, int audioID )
{
    if ( !file ) return -1;

    /* when changing audio/video stream, streamMutex should be locked */
    SDL_LockMutex( file->streamMutex );

    /* check if we have any audiostreams and if the requested ID is available */
    if ( !file->audioStreams || audioID >= ( int )file->audioStreams )
    {
        SDL_UnlockMutex( file->streamMutex );

        SDL_ffmpegSetError( "requested audio stream ID is not available in file" );
        return -1;
    }

    /* set all audio streams to discard */
    SDL_ffmpegStream *stream = file->as;

    /* discard by default */
    while ( stream && stream->_ffmpeg )
    {
        stream->_ffmpeg->discard = AVDISCARD_ALL;
        stream = stream->next;
    }

    if ( audioID < 0 )
    {
        /* reset audiostream */
        file->audioStream = 0;
    }
    else
    {
        /* set current audiostream to stream linked to audioID */
        file->audioStream = file->as;

        for ( int i = 0; i < audioID && file->audioStream; i++ ) file->audioStream = file->audioStream->next;

        /* active stream need not be discarded */
		if (file->audioStream && file->audioStream->_ffmpeg)
			file->audioStream->_ffmpeg->discard = AVDISCARD_DEFAULT;
    }

    SDL_UnlockMutex( file->streamMutex );

    return 0;
}


/** \brief  Get the desired video stream from file.

            This returns a pointer to the requested stream. With this stream pointer you can
            get information about the stream, like language, samplerate, size etc.
            Based on this information you can choose the stream you want to use.
\param      file SDL_ffmpegFile from which the information is required
\param      videoID is the stream you whish to select.
\returns    Pointer to SDL_ffmpegStream, or NULL if selected stream could not be found
*/
SDL_ffmpegStream* SDL_ffmpegGetVideoStream( SDL_ffmpegFile *file, uint32_t videoID )
{
    /* check if we have any audiostreams */
    if ( !file || !file->videoStreams ) return 0;

    /* check if the requested id is possible */
    if ( videoID >= file->videoStreams ) return 0;

    SDL_ffmpegStream *s = file->vs;

    /* return audiostream linked to audioID */
    for ( uint32_t i = 0; i < videoID && s; i++ ) s = s->next;

    return s;
}


/** \brief  Select a video stream from file.

            Use this function to select a video stream for decoding.
            Using SDL_ffmpegGetVideoStream you can get information about the streams.
            Based on that you can chose the stream you want.
\param      file SDL_ffmpegFile on which an action is required
\param      videoID is the stream you whish to select.
\returns    -1 on error, otherwise 0
*/
int SDL_ffmpegSelectVideoStream( SDL_ffmpegFile* file, int videoID )
{
    if ( !file ) return -1;

    /* when changing audio/video stream, streamMutex should be locked */
    SDL_LockMutex( file->streamMutex );

    /* check if we have any videostreams */
    if ( videoID >= ( int )file->videoStreams )
    {
        SDL_UnlockMutex( file->streamMutex );

        SDL_ffmpegSetError( "requested video stream ID is not available in file" );

        return -1;
    }

    /* set all video streams to discard */
    SDL_ffmpegStream *stream = file->vs;

    /* discard by default */
    while ( stream && stream->_ffmpeg )
    {
        stream->_ffmpeg->discard = AVDISCARD_ALL;
        stream = stream->next;
    }

    if ( videoID < 0 )
    {
        /* reset videostream */
        file->videoStream = 0;
    }
    else
    {
        /* set current videostream to stream linked to videoID */
        file->videoStream = file->vs;

        /* keep searching for correct videostream */
        for ( int i = 0; i < videoID && file->videoStream; i++ ) file->videoStream = file->videoStream->next;

        /* active stream need not be discarded */
		if (file->videoStream && file->videoStream->_ffmpeg)
			file->videoStream->_ffmpeg->discard = AVDISCARD_DEFAULT;
    }

    SDL_UnlockMutex( file->streamMutex );

    return 0;
}

/** \brief  Seek to a certain point in file.

            Tries to seek to specified point in file.
\param      file SDL_ffmpegFile on which an action is required
\param      timestamp is represented in milliseconds.
\returns    -1 on error, otherwise 0
*/
int SDL_ffmpegSeek( SDL_ffmpegFile* file, uint64_t timestamp )
{
    if ( !file ) return -1;

    if ( SDL_ffmpegDuration( file ) < timestamp )
    {
        SDL_ffmpegSetError( "can not seek past end of file" );

        return -1;
    }

    /* convert milliseconds to AV_TIME_BASE units */
    uint64_t seekPos = timestamp * ( AV_TIME_BASE / 1000 );

    /* AVSEEK_FLAG_BACKWARD means we jump to the first keyframe before seekPos */
    av_seek_frame( file->_ffmpeg, -1, seekPos, AVSEEK_FLAG_BACKWARD );

    /* set minimal timestamp to decode */
    file->minimalTimestamp = timestamp;

    /* flush buffers */
    SDL_ffmpegFlush( file );

    return 0;
}

/** \brief  Seek to a relative point in file.

            Tries to seek to new location, based on current location in file.
\param      file SDL_ffmpegFile on which an action is required
\param      timestamp is represented in milliseconds.
\returns    -1 on error, otherwise 0
*/
int SDL_ffmpegSeekRelative( SDL_ffmpegFile *file, int64_t timestamp )
{
    /* same thing as normal seek, just take into account the current position */
    return SDL_ffmpegSeek( file, SDL_ffmpegGetPosition( file ) + timestamp );
}

/**
\cond
*/
int SDL_ffmpegFlush( SDL_ffmpegFile *file )
{
    /* check for file and permission to flush buffers */
    if ( !file ) return -1;

    /* when accesing audio/video stream, streamMutex should be locked */
    SDL_LockMutex( file->streamMutex );

    /* if we have a valid audio stream, we flush it */
    if ( file->audioStream )
    {
        SDL_LockMutex( file->audioStream->mutex );

        SDL_ffmpegPacket *pack = file->audioStream->buffer;

        while ( pack )
        {
            SDL_ffmpegPacket *old = pack;

            pack = pack->next;

            av_packet_free( &old->data );

            free( old );
        }

        file->audioStream->buffer = 0;

        /* flush internal ffmpeg buffers */
        if ( file->audioStream->_ffmpeg )
        {
            avcodec_flush_buffers( file->audioStream->_ctx );
        }

        SDL_UnlockMutex( file->audioStream->mutex );
    }

    /* if we have a valid video stream, we flush some more */
    if ( file->videoStream )
    {
        SDL_LockMutex( file->videoStream->mutex );

        SDL_ffmpegPacket *pack = file->videoStream->buffer;

        while ( pack )
        {
            SDL_ffmpegPacket *old = pack;

            pack = pack->next;

            av_packet_free( &old->data );

            free( old );
        }

        file->videoStream->buffer = 0;

        /* flush internal ffmpeg buffers */
        if ( file->videoStream->_ffmpeg ) avcodec_flush_buffers( file->videoStream->_ctx );

        SDL_UnlockMutex( file->videoStream->mutex );
    }

    SDL_UnlockMutex( file->streamMutex );

    return 0;
}
/**
\endcond
*/

/** \brief  Use this to get a pointer to a SDL_ffmpegAudioFrame.

            If you receive a frame, it is valid until you receive a new frame, or
            until the file is freed, using SDL_ffmpegFree( SDL_ffmpegFile* ).
            I you use data from the frame, you should adjust the size member by
            the amount of data used in bytes. This is needed so that SDL_ffmpeg can
            calculate the next frame.
\param      file SDL_ffmpegFile from which the information is required
\param      frame The frame to which the data will be decoded.
\returns    Pointer to SDL_ffmpegAudioFrame, or NULL if no frame was available.
*/
int SDL_ffmpegGetAudioFrame( SDL_ffmpegFile *file, SDL_ffmpegAudioFrame *frame )
{
    if ( !file || !frame ) return -1;

    /* when accesing audio/video stream, streamMutex should be locked */
    SDL_LockMutex( file->streamMutex );

    if ( !file->audioStream )
    {
        SDL_UnlockMutex( file->streamMutex );

        SDL_ffmpegSetError( "no valid audio stream selected" );
        return 0;
    }

    /* lock audio buffer */
    SDL_LockMutex( file->audioStream->mutex );

    /* reset frame end pointer and size */
    frame->last = 0;
    frame->size = 0;

    /* get new packet */
    SDL_ffmpegPacket *pack = SDL_ffmpegGetAudioPacket( file );

    while ( !pack && !frame->last )
    {
        pack = SDL_ffmpegGetAudioPacket( file );

        frame->last = SDL_ffmpegGetPacket( file );
    }

    /* SDL_ffmpegDecodeAudioFrame will return true if data from pack was used
       frame will be updated with the new data */
    while ( pack && SDL_ffmpegDecodeAudioFrame( file, pack->data, frame ) )
    {
        /* destroy used packet */
        av_packet_free( &pack->data );

        free( pack );

        pack = 0;

        /* check if new packet is required */
        if ( frame->size < frame->capacity )
        {
            /* try to get a new packet */
            pack = SDL_ffmpegGetAudioPacket( file );

            while ( !pack && !frame->last )
            {
                pack = SDL_ffmpegGetAudioPacket( file );

                frame->last = SDL_ffmpegGetPacket( file );
            }
        }
    }

    /* pack retreived, but was not used, push it back in the buffer */
    if ( pack )
    {
        /* take current buffer as next pointer */
        pack->next = file->audioStream->buffer;

        /* store pack as current buffer */
        file->audioStream->buffer = pack;
    }

    /* unlock audio buffer */
    SDL_UnlockMutex( file->audioStream->mutex );

    SDL_UnlockMutex( file->streamMutex );

    return ( frame->size == frame->capacity );
}


/** \brief  Returns the current position of the file in milliseconds.

\param      file SDL_ffmpegFile from which the information is required
\returns    -1 on error, otherwise the length of the file in milliseconds
*/
int64_t SDL_ffmpegGetPosition( SDL_ffmpegFile *file )
{
    if ( !file ) return -1;

    /* when accesing audio/video stream, streamMutex should be locked */
    SDL_LockMutex( file->streamMutex );

    int64_t pos = 0;

    if ( file->audioStream )
    {
        pos = file->audioStream->lastTimeStamp;
    }

    if ( file->videoStream && file->videoStream->lastTimeStamp > pos )
    {
        pos = file->videoStream->lastTimeStamp;
    }

    SDL_UnlockMutex( file->streamMutex );

    /* return the current playposition of our file */
    return pos;
}


/** \brief  Returns the frame rate of the stream as a fraction.

            This retreives the frame rate of the supplied stream.
            For example, a framerate of 25 frames per second will have a nominator
            of 1, and a denominator of 25.
\param      stream SDL_ffmpegStream from which the information is required.
\param      nominator Nominator part of the fraction. Can be 0 if exact value of
                      nominator is not required.
\param      denominator Denominator part of the fraction. Can be 0 if exact value
                        of denominator is not required.
\returns    The result of nominator / denominator as floating point value.
*/
float SDL_ffmpegGetFrameRate( SDL_ffmpegStream *stream, int *nominator, int *denominator )
{
    if ( stream && stream->_ctx )
    {
        AVRational frate = stream->_ffmpeg->avg_frame_rate;
        if ( nominator ) *nominator = frate.num;

        if ( denominator ) *denominator = frate.den;

        return ( float )frate.num / frate.den;
    }
    else
    {
        SDL_ffmpegSetError( "could not retreive frame rate from stream" );

        if ( nominator ) *nominator = 0;

        if ( denominator ) *denominator = 0;
    }

    return 0.0;
}

/** \brief  This can be used to get a SDL_AudioSpec based on values found in file

            This returns a SDL_AudioSpec, if you have selected a valid audio
            stream. Otherwise, all values are set to NULL.
\param      file SDL_ffmpegFile from which the information is required
\param      samples Amount of samples required every time the callback is called.
            Lower values mean less latency, but please note that SDL has a minimal value.
\param      callback Pointer to callback function
\returns    SDL_AudioSpec with values set according to the selected audio stream.
            If no valid audio stream was available, all values of returned SDL_AudioSpec are set to 0
*/
SDL_AudioSpec SDL_ffmpegGetAudioSpec( SDL_ffmpegFile *file, uint16_t samples, SDL_ffmpegCallback callback )
{
    /* create audio spec */
    SDL_AudioSpec spec;

    memset( &spec, 0, sizeof( SDL_AudioSpec ) );

    if ( !file ) return spec;

    /* when accesing audio/video stream, streamMutex should be locked */
    SDL_LockMutex( file->streamMutex );

    /* if we have a valid audiofile, we can use its data to create a
       more appropriate audio spec */
    if ( file->audioStream )
    {
        spec.format = AUDIO_S16SYS;
        spec.samples = samples;
        spec.userdata = file;
        spec.callback = callback;
        spec.freq = file->audioStream->_ctx->sample_rate;
        spec.channels = ( uint8_t )file->audioStream->_ctx->channels;
    }
    else
    {
        SDL_ffmpegSetError( "no valid audio stream selected" );
    }

    SDL_UnlockMutex( file->streamMutex );

    return spec;
}


/** \brief  Returns the Duration of the file in milliseconds.

            Please note that this value is guestimated by FFmpeg, it may differ from
            actual playing time.
\param      file SDL_ffmpegFile from which the information is required
\returns    -1 on error, otherwise the length of the file in milliseconds
*/
uint64_t SDL_ffmpegDuration( SDL_ffmpegFile *file )
{
    if ( !file ) return 0;

    if ( file->type == SDL_ffmpegInputStream )
    {
        /* returns the duration of the entire file, please note that ffmpeg doesn't
           always get this value right! so don't bet your life on it... */
        return file->_ffmpeg->duration / ( AV_TIME_BASE / 1000 );
    }

    if ( file->type == SDL_ffmpegOutputStream )
    {
        uint64_t v = SDL_ffmpegVideoDuration( file );
        uint64_t a = SDL_ffmpegAudioDuration( file );

        if ( v > a ) return v;
        return a;
    }

    return 0;
}


/** \brief  Returns the duration of the audio stream in milliseconds.

            This value can be used to sync two output streams.
\param      file SDL_ffmpegFile from which the information is required
\returns    -1 on error, otherwise the length of the file in milliseconds
*/
uint64_t SDL_ffmpegAudioDuration( SDL_ffmpegFile *file )
{
    if ( !file ) return 0;

    /* when accesing audio/video stream, streamMutex should be locked */
    SDL_LockMutex( file->streamMutex );

    uint64_t duration = 0;

    if ( file->audioStream )
    {
        if ( file->type == SDL_ffmpegInputStream )
        {
            duration = av_rescale( 1000 * file->audioStream->_ffmpeg->duration, file->audioStream->_ffmpeg->time_base.num, file->audioStream->_ffmpeg->time_base.den );
        }
        else if ( file->type == SDL_ffmpegOutputStream )
        {
            duration = file->audioStream->frameCount * file->audioStream->encodeAudioInputSize / ( file->audioStream->_ctx->sample_rate / 1000 );
        }
    }
    else
    {
        SDL_ffmpegSetError( "no valid audio stream selected" );
    }

    SDL_UnlockMutex( file->streamMutex );

    return duration;
}


/** \brief  Returns the duration of the video stream in milliseconds.

            This value can be used to sync two output streams.
\param      file SDL_ffmpegFile from which the information is required
\returns    -1 on error, otherwise the length of the file in milliseconds
*/
uint64_t SDL_ffmpegVideoDuration( SDL_ffmpegFile *file )
{
    if ( !file ) return 0;

    /* when accesing audio/video stream, streamMutex should be locked */
    SDL_LockMutex( file->streamMutex );

    uint64_t duration = 0;

    if ( file->videoStream )
    {
        if ( file->type == SDL_ffmpegInputStream )
        {
            duration = av_rescale( 1000 * file->videoStream->_ffmpeg->duration, file->videoStream->_ffmpeg->time_base.num, file->videoStream->_ffmpeg->time_base.den );
        }
        else if ( file->type == SDL_ffmpegOutputStream )
        {
            duration = av_rescale( 1000 * file->videoStream->frameCount, file->videoStream->_ctx->time_base.num, file->videoStream->_ctx->time_base.den );
        }
    }
    else
    {
        SDL_ffmpegSetError( "no valid video stream selected" );
    }

    SDL_UnlockMutex( file->streamMutex );

    return duration;
}

/** \brief  retreive the width/height of a frame beloning to file

            With this function you can get the width and height of a frame, belonging to
            your file. If there is no (valid) videostream selected w and h default
            to 0. Please not that you will have to make sure the pointers are
            allocated.

\param      file SDL_ffmpegFile from which the information is required
\param      w width
\param      h height
\returns    -1 on error, otherwise 0
*/
int SDL_ffmpegGetVideoSize( SDL_ffmpegFile *file, int *w, int *h )
{
    if ( !file || !w || !h ) return -1;

    /* when accesing audio/video stream, streamMutex should be locked */
    SDL_LockMutex( file->streamMutex );

    /* if we have a valid video file selected, we use it
       if not, we send default values and return.
       by checking the return value you can check if you got a valid size */
    if ( file->videoStream )
    {
        *w = file->videoStream->_ctx->width;
        *h = file->videoStream->_ctx->height;

        SDL_UnlockMutex( file->streamMutex );

        return 0;
    }
    else
    {
        SDL_ffmpegSetError( "no valid video stream selected" );
    }

    *w = 0;
    *h = 0;

    SDL_UnlockMutex( file->streamMutex );

    return -1;
}


/** \brief  This is used to check if a valid audio stream is selected.

\param      file SDL_ffmpegFile from which the information is required
\returns    non-zero if a valid video stream is selected, otherwise 0
*/
int SDL_ffmpegValidAudio( SDL_ffmpegFile* file )
{
    /* this function is used to check if we selected a valid audio stream */
    return ( file && file->audioStream );
}


/** \brief  This is used to check if a valid video stream is selected.

\param      file SDL_ffmpegFile from which the information is required
\returns    non-zero if a valid video stream is selected, otherwise 0
*/
int SDL_ffmpegValidVideo( SDL_ffmpegFile* file )
{
    /* this function is used to check if we selected a valid video stream */
    return ( file && file->videoStream );
}


/** \brief  This is used to add a video stream to file

\param      file SDL_ffmpegFile to which the stream will be added
\param      codec SDL_ffmpegCodec describing the type encoding to be used by this
                  stream.
\returns    The stream which was added, or NULL if no stream could be added.
*/
SDL_ffmpegStream* SDL_ffmpegAddVideoStream( SDL_ffmpegFile *file, SDL_ffmpegCodec codec )
{
    /* add a video stream */
    const AVCodec* videoCodec = avcodec_find_encoder(codec.videoCodecID);
    if (!videoCodec)
    {
        SDL_ffmpegSetError("video codec not found");
        return 0;
    }
    AVStream* stream = avformat_new_stream(file->_ffmpeg, videoCodec);
    if (!stream)
    {
        SDL_ffmpegSetError("video stream could not be created");
        return 0;
    }
    AVCodecContext* context = avcodec_alloc_context3(videoCodec);
    if (!context)
    {
        SDL_ffmpegSetError("video codec context alloc error");
        return 0;
    }

    stream->codecpar->codec_id = codec.videoCodecID;
    stream->codecpar->codec_type = AVMEDIA_TYPE_VIDEO;

    stream->codecpar->bit_rate = codec.videoBitrate;

    /* resolution must be a multiple of two */
    stream->codecpar->width = codec.width;
    stream->codecpar->height = codec.height;

    /* set pixel format */
    stream->codecpar->format = AV_PIX_FMT_YUV420P;

    if (avcodec_parameters_to_context(context, stream->codecpar) < 0)
    {
        SDL_ffmpegSetError("codec parameters load error");
        return 0;
    }

    AVRational rational = { codec.framerateNum, codec.framerateDen };
    context->time_base = rational;

    context->qmax = codec.videoMaxRate;
    context->qmin = codec.videoMinRate;

    context->thread_count = codec.cpuCount;
    context->flags |= AV_CODEC_FLAG_CLOSED_GOP;

    av_opt_set(context->priv_data, "crf", codec.crf, 0);

    /* some formats want stream headers to be separate */
    if ( file->_ffmpeg->oformat->flags & AVFMT_GLOBALHEADER )
    {
        file->_ffmpeg->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    /* open the codec */
    if ( avcodec_open2(context, 0, 0 ) < 0 || avcodec_parameters_from_context(stream->codecpar, context) < 0)
    {
        SDL_ffmpegSetError( "could not open video codec" );
        return 0;
    }

    /* create a new stream */
    SDL_ffmpegStream *str = ( SDL_ffmpegStream* )malloc( sizeof( SDL_ffmpegStream ) );

    if ( str )
    {
        /* we set our stream to zero */
        memset( str, 0, sizeof( SDL_ffmpegStream ) );

        str->id = file->videoStreams;

        /* _ffmpeg holds data about streamcodec */
        str->_ffmpeg = stream;
        str->_ctx = context;

        str->mutex = SDL_CreateMutex();

        str->encodeFrame = av_frame_alloc();

        int size = av_image_get_buffer_size( stream->codecpar->format, stream->codecpar->width, stream->codecpar->height, 1 );
        uint8_t* picture_buf = ( uint8_t* )av_malloc( size + AV_INPUT_BUFFER_PADDING_SIZE);
        av_image_fill_arrays(str->encodeFrame->data, str->encodeFrame->linesize, picture_buf, stream->codecpar->format, stream->codecpar->width, stream->codecpar->height, 1 );

        str->encodeFrameBufferSize = stream->codecpar->width * stream->codecpar->height * 4 + AV_INPUT_BUFFER_PADDING_SIZE + 10000;

        str->encodeFrameBuffer = ( uint8_t* )av_malloc( str->encodeFrameBufferSize );

        file->videoStreams++;

        /* find correct place to save the stream */
        SDL_ffmpegStream **s = &file->vs;
        while ( *s )
        {
            *s = ( *s )->next;
        }

        *s = str;
    }

    return str;
}


/** \brief  This is used to add a video stream to file

\param      file SDL_ffmpegFile to which the stream will be added
\param      codec SDL_ffmpegCodec describing the type encoding to be used by this
                  stream.
\returns    The stream which was added, or NULL if no stream could be added.
*/
SDL_ffmpegStream* SDL_ffmpegAddAudioStream( SDL_ffmpegFile *file, SDL_ffmpegCodec codec )
{
    // add an audio stream
    const AVCodec* audioCodec = avcodec_find_encoder(codec.audioCodecID);
    if (!audioCodec)
    {
        SDL_ffmpegSetError("audio codec not found");
        return 0;
    }
    AVStream* stream = avformat_new_stream(file->_ffmpeg, audioCodec);
    if (!stream)
    {
        SDL_ffmpegSetError("audio stream could not be created");
        return 0;
    }
    AVCodecContext* context = avcodec_alloc_context3(audioCodec);
    if (!context)
    {
        SDL_ffmpegSetError("audio codec context alloc error");
        return 0;
    }

    stream->codecpar->codec_id = codec.audioCodecID;
    stream->codecpar->codec_type = AVMEDIA_TYPE_AUDIO;
    stream->codecpar->sample_rate = codec.sampleRate;
    stream->codecpar->format = AV_SAMPLE_FMT_FLTP;
    stream->codecpar->channels = codec.channels;
    stream->codecpar->channel_layout = codec.channels == 2 ? AV_CH_LAYOUT_STEREO : AV_CH_LAYOUT_MONO;

    if (avcodec_parameters_to_context(context, stream->codecpar) < 0)
    {
        SDL_ffmpegSetError("codec parameters load error");
        return 0;
    }

    AVRational rational = { 1, codec.sampleRate };
    context->time_base = rational;

    context->strict_std_compliance = FF_COMPLIANCE_EXPERIMENTAL;
    context->global_quality = FF_QP2LAMBDA * (codec.audioQuality / 10);
    context->flags |= AV_CODEC_FLAG_QSCALE;

    // open the codec
    if (avcodec_open2(context, 0, 0) < 0 || avcodec_parameters_from_context(stream->codecpar, context) < 0)
    {
        SDL_ffmpegSetError("could not open audio codec");
        return 0;
    }

    /* some formats want stream headers to be separate */
    if ( file->_ffmpeg->oformat->flags & AVFMT_GLOBALHEADER )
    {
        file->_ffmpeg->flags |= AV_CODEC_FLAG_GLOBAL_HEADER;
    }

    /* create a new stream */
    SDL_ffmpegStream *str = ( SDL_ffmpegStream* )malloc( sizeof( SDL_ffmpegStream ) );

    if ( str )
    {
        /* we set our stream to zero */
        memset( str, 0, sizeof( SDL_ffmpegStream ) );

        str->id = file->audioStreams;

        str->encodeFrame = av_frame_alloc();

        /* _ffmpeg holds data about streamcodec */
        str->_ffmpeg = stream;
        str->_ctx = context;
        str->audioFormat = codec.audioFormat;

        // init resampler
        str->swr_context = swr_alloc_set_opts(str->swr_context, context->channel_layout, context->sample_fmt, context->sample_rate,
            context->channel_layout, str->audioFormat, context->sample_rate, 0, NULL);

        if (!str->swr_context || swr_init(str->swr_context) < 0)
        {
            SDL_ffmpegSetError("could not initialize resampler");
            return 0;
        }

        str->mutex = SDL_CreateMutex();

        str->sampleBufferSize = av_samples_get_buffer_size(0, stream->codecpar->channels, stream->codecpar->frame_size, AV_SAMPLE_FMT_FLT, 0);

        if (av_samples_alloc((uint8_t**)(&str->sampleBuffer), 0, stream->codecpar->channels, stream->codecpar->frame_size, AV_SAMPLE_FMT_FLT, 0) < 0)
        {
            SDL_ffmpegSetError("could not allocate samples for audio buffer");
            return 0;
        }

        /* ugly hack for PCM codecs (will be removed ASAP with new PCM
           support to compute the input frame size in samples */
        if ( stream->codecpar->frame_size <= 1 )
        {
            str->encodeAudioInputSize = str->sampleBufferSize / stream->codecpar->channels;

            switch ( stream->codecpar->codec_id )
            {
                case AV_CODEC_ID_PCM_S16LE:
                case AV_CODEC_ID_PCM_S16BE:
                case AV_CODEC_ID_PCM_U16LE:
                case AV_CODEC_ID_PCM_U16BE:
                    str->encodeAudioInputSize >>= 1;
                    break;
                default:
                    break;
            }
        }
        else
        {
            str->encodeAudioInputSize = stream->codecpar->frame_size;
        }

        file->audioStreams++;

        /* find correct place to save the stream */
        SDL_ffmpegStream **s = &file->as;
        while ( *s )
        {
            *s = ( *s )->next;
        }

        *s = str;
    }

    return str;
}

/** \brief  Use this function to query if an error occured

\returns    non-zero when an error occured
*/
int SDL_ffmpegError()
{
    return SDL_ffmpegErrorMessage[ 0 ];
}


/** \brief  Use this function to get the last error string

\returns    When no error was found, NULL is returned
*/
const char* SDL_ffmpegGetError()
{
    return SDL_ffmpegErrorMessage;
}


/** \brief  Use this function to clear all standing errors

*/
void SDL_ffmpegClearError()
{
    SDL_ffmpegErrorMessage[ 0 ] = 0;
}

/**
\cond
*/

void SDL_ffmpegSetError( const char *error )
{
    if ( !error ) return;

    if ( snprintf( SDL_ffmpegErrorMessage, 512, "%s", error ) >= 511 )
    {
        SDL_ffmpegErrorMessage[ 511 ] = 0;
    }
}

int SDL_ffmpegGetPacket( SDL_ffmpegFile *file )
{
    /* entering this function, streamMutex should have been locked */

    /* create a packet for our data */
    AVPacket *pack = av_packet_alloc();

    if (!pack) return 0;

    /* read a packet from the file */
    int decode = av_read_frame( file->_ffmpeg, pack );

    /* if we did not get a packet, we probably reached the end of the file */
    if ( decode < 0 )
    {
        av_packet_free( &pack );

        /* signal EOF */
        return 1;
    }

    /* we got a packet, lets handle it */

    /* If it's a packet from either of our streams, return it */
    if ( file->audioStream && pack->stream_index == file->audioStream->id )
    {
        /* prepare packet */
        SDL_ffmpegPacket *temp = ( SDL_ffmpegPacket* )malloc( sizeof( SDL_ffmpegPacket ) );
        // TODO check and handle the case where temp failed to malloc
        temp->data = pack;
        temp->next = 0;

        SDL_ffmpegPacket **p = &file->audioStream->buffer;

        while ( *p )
        {
            p = &( *p )->next;
        }

        *p = temp;
    }
    else if ( file->videoStream && pack->stream_index == file->videoStream->id )
    {
        /* prepare packet */
        SDL_ffmpegPacket *temp = ( SDL_ffmpegPacket* )malloc( sizeof( SDL_ffmpegPacket ) );
        temp->data = pack;
        temp->next = 0;

//            SDL_LockMutex( file->videoStream->mutex );

        SDL_ffmpegPacket **p = &file->videoStream->buffer;

        while ( *p )
        {
            p = &( *p )->next;
        }

        *p = temp;

//            SDL_UnlockMutex( file->videoStream->mutex );
    }
    else
    {
        av_packet_unref( pack );
    }

    return 0;
}

SDL_ffmpegPacket* SDL_ffmpegGetAudioPacket( SDL_ffmpegFile *file )
{
    if ( !file->audioStream ) return 0;

    /* file->audioStream->mutex should be locked before entering this function */

    SDL_ffmpegPacket *pack = 0;

    /* check if there are still packets in buffer */
    if ( file->audioStream->buffer )
    {
        pack = file->audioStream->buffer;

        file->audioStream->buffer = pack->next;
    }

    /* if a packet was found, return it */
    return pack;
}

SDL_ffmpegPacket* SDL_ffmpegGetVideoPacket( SDL_ffmpegFile *file )
{
    if ( !file->videoStream ) return 0;

    /* file->videoStream->mutex should be locked before entering this function */

    SDL_ffmpegPacket *pack = 0;

    /* check if there are still packets in buffer */
    if ( file->videoStream->buffer )
    {
        pack = file->videoStream->buffer;

        file->videoStream->buffer = pack->next;
    }

    /* if a packet was found, return it */
    return pack;
}

int SDL_ffmpegDecodeAudioFrame( SDL_ffmpegFile *file, AVPacket *pack, SDL_ffmpegAudioFrame *frame )
{
    int audioSize = AVCODEC_MAX_AUDIO_FRAME_SIZE * sizeof( float );

    int channels = file->audioStream->_ctx->channels;
    enum AVSampleFormat format = AV_SAMPLE_FMT_FLT;
    int bps = av_get_bytes_per_sample(format);

    /* check if there is still data in the buffer */
    if ( file->audioStream->sampleBufferSize )
    {
        /* set new pts */
        if ( !frame->size ) frame->pts = file->audioStream->sampleBufferTime;

        /* calculate free space in frame */
        int fs = frame->capacity - frame->size;

        /* check the amount of data which needs to be copied */
        int in_samples = file->audioStream->sampleBufferSize / (channels * bps);
        int out_samples = fs / (channels * bps);
        
        if (out_samples < in_samples)
        {
            /* copy data from sampleBuffer into frame buffer until frame buffer is full */
            int size = out_samples * channels * bps;
            memcpy(frame->buffer + frame->size, file->audioStream->sampleBuffer + file->audioStream->sampleBufferOffset, size);
            
            /* mark the amount of bytes still in the buffer */
            file->audioStream->sampleBufferSize -= out_samples * channels * bps;

            /* move offset accordingly */
            if (av_sample_fmt_is_planar(format))
                file->audioStream->sampleBufferOffset += out_samples * bps;
            else
                file->audioStream->sampleBufferOffset += out_samples * bps * channels;

            /* update framesize */
            frame->size += size;
        }
        else
        {
            /* copy data from sampleBuffer into frame buffer until sampleBuffer is empty */
            int size = in_samples * channels * bps;
            memcpy(frame->buffer + frame->size, file->audioStream->sampleBuffer + file->audioStream->sampleBufferOffset, size);

            /* update framesize */
            frame->size += size;

            /* at this point, samplebuffer should have been handled */
            file->audioStream->sampleBufferSize = 0;

            /* no more data in buffer, reset offset */
            file->audioStream->sampleBufferOffset = 0;
        }

        /* return 0 to signal caller that 'pack' was not used */
        if ( frame->size == frame->capacity ) return 0;
    }


    /* calculate pts to determine wheter or not this frame should be stored */
    file->audioStream->sampleBufferTime = av_rescale(( pack->dts - file->audioStream->_ffmpeg->start_time ) * 1000, file->audioStream->_ffmpeg->time_base.num, file->audioStream->_ffmpeg->time_base.den );

    /* Decode the packet */
    AVCodecContext *avctx = file->audioStream->_ctx;
    AVFrame* dframe = file->audioStream->decodeFrame;

    int len = avcodec_send_packet( avctx, pack );
        
    if (len < 0)
    {
        SDL_ffmpegSetError( "error decoding audio frame" );
        return 0;
    }

    AVFrame* convertedFrame = file->audioStream->encodeFrame;

    while (avcodec_receive_frame(avctx, dframe) == 0) {

        dframe->channel_layout |= dframe->channels == 2 ? AV_CH_LAYOUT_STEREO : AV_CH_LAYOUT_MONO; 
        convertedFrame->nb_samples = dframe->nb_samples;
        convertedFrame->channel_layout = dframe->channel_layout;
        convertedFrame->sample_rate = dframe->sample_rate;
        convertedFrame->format = AV_SAMPLE_FMT_FLT;

        if (swr_convert_frame(file->audioStream->swr_context, convertedFrame, dframe) < 0)
        {
            SDL_ffmpegSetError("can't convert audio frame");
            break;
        }

        int planar = av_sample_fmt_is_planar(convertedFrame->format);
        int plane_size;

        int data_size = av_samples_get_buffer_size(&plane_size, convertedFrame->channels, convertedFrame->nb_samples, convertedFrame->format, 1);

        memcpy(file->audioStream->sampleBuffer, convertedFrame->extended_data[0], plane_size);
        audioSize = plane_size;
        if (planar && convertedFrame->channels > 1)
        {
            int8_t* out = file->audioStream->sampleBuffer + plane_size;
            int ch;
            for (ch = 1; ch < convertedFrame->channels; ch++)
            {
                memcpy(out, convertedFrame->extended_data[ch], plane_size);
                out += plane_size;
                audioSize += plane_size;
            }
        }

        {
            /* set new pts */
            if (!frame->size) frame->pts = file->audioStream->sampleBufferTime;

            /* room in frame */
            int fs = frame->capacity - frame->size;

            /* check if there is room at all */
            if (fs)
            {
                /* check the amount of data which needs to be copied */
                int in_samples = audioSize / (channels * bps);
                int out_samples = fs / (channels * bps);

                if (out_samples < in_samples)
                {
                    /* copy data from sampleBuffer into frame buffer until frame buffer is full */
                    int size = out_samples * channels * bps;
                    memcpy(frame->buffer + frame->size, file->audioStream->sampleBuffer, size);

                    /* mark the amount of bytes still in the buffer */
                    file->audioStream->sampleBufferSize = ((in_samples - out_samples) * channels * bps);

                    /* set the offset so the remaining data can be found */
                    if (av_sample_fmt_is_planar(format))
                        file->audioStream->sampleBufferOffset = out_samples * bps;
                    else
                        file->audioStream->sampleBufferOffset = out_samples * bps * channels;

                    /* update framesize */
                    frame->size += size;
                }
                else
                {
                    /* copy data from sampleBuffer into frame buffer until sampleBuffer is empty */
                    int size = in_samples * channels * bps;
                    memcpy(frame->buffer + frame->size, file->audioStream->sampleBuffer, size);

                    /* mark the amount of bytes still in the buffer */
                    file->audioStream->sampleBufferSize = 0;

                    /* reset buffer offset */
                    file->audioStream->sampleBufferOffset = 0;

                    /* update framesize */
                    frame->size += size;
                }
            }
            else
            {
                /* no room in frame, mark samplebuffer as full */
                file->audioStream->sampleBufferSize = audioSize;

                /* reset buffer offset */
                file->audioStream->sampleBufferOffset = 0;
            }
        }

        av_frame_unref(dframe);
        av_frame_unref(convertedFrame);
    }

    /* pack was used, return 1 */
    return 1;
}

int SDL_ffmpegDecodeVideoFrame( SDL_ffmpegFile* file, AVPacket *pack, SDL_ffmpegVideoFrame *frame )
{
    int got_frame = 0;

    if ( pack )
    {
        /* usefull when dealing with B frames */
        if ( pack->dts == AV_NOPTS_VALUE )
        {
            /* if we did not get a valid timestamp, we make one up based on the last
               valid timestamp + the duration of a frame */
            frame->pts = file->videoStream->lastTimeStamp + av_rescale( 1000 * pack->duration, file->videoStream->_ffmpeg->time_base.num, file->videoStream->_ffmpeg->time_base.den );
        }
        else
        {
            /* write timestamp into the buffer */
            frame->pts = av_rescale(( pack->dts - file->videoStream->_ffmpeg->start_time ) * 1000, file->videoStream->_ffmpeg->time_base.num, file->videoStream->_ffmpeg->time_base.den );
        }

        /* Decode the packet */
        if (avcodec_send_packet(file->videoStream->_ctx, pack) == 0)
        {
            got_frame = avcodec_receive_frame(file->videoStream->_ctx, file->videoStream->decodeFrame) == 0;
        }
    }
    else
    {
        /* check if there is still a frame left in the buffer */
        if (avcodec_send_packet(file->videoStream->_ctx, NULL) == 0)
        {
            got_frame = avcodec_receive_frame(file->videoStream->_ctx, file->videoStream->decodeFrame) == 0;
        }
    }

    /* if we did not get a frame, we return */
    if ( got_frame )
    {
        /* convert YUV 420 to YUYV 422 data */
//        if ( frame->overlay && frame->overlay->format == SDL_YUY2_OVERLAY )
//        {
//            int pitch[] =
//            {
//                frame->overlay->pitches[ 0 ],
//                frame->overlay->pitches[ 1 ],
//                frame->overlay->pitches[ 2 ]
//            };
//
//            sws_scale( getContext( &file->videoStream->conversionContext,
//                                   file->videoStream->_ffmpeg->codec->width,
//                                   file->videoStream->_ffmpeg->codec->height,
//                                   file->videoStream->_ffmpeg->codec->pix_fmt,
//                                   frame->overlay->w, frame->overlay->h,
//                                   AV_PIX_FMT_YUYV422 ),
//                       ( const uint8_t* const* )file->videoStream->decodeFrame->data,
//                       file->videoStream->decodeFrame->linesize,
//                       0,
//                       file->videoStream->_ffmpeg->codec->height,
//                       ( uint8_t* const* )frame->overlay->pixels,
//                       pitch );
//        }

        /* convert YUV to RGB data */
        if ( frame->surface && frame->surface->format )
        {
            int pitch = frame->surface->pitch;

            switch ( frame->surface->format->BitsPerPixel )
            {
                case 32:
                    sws_scale( getContext( &file->videoStream->conversionContext,
                                           file->videoStream->_ctx->width,
                                           file->videoStream->_ctx->height,
                                           file->videoStream->_ctx->pix_fmt,
                                           frame->surface->w, frame->surface->h,
                                           AV_PIX_FMT_RGB32 ),
                               ( const uint8_t* const* )file->videoStream->decodeFrame->data,
                               file->videoStream->decodeFrame->linesize,
                               0,
                               file->videoStream->_ctx->height,
                               ( uint8_t* const* )&frame->surface->pixels,
                               &pitch );
                    break;
                case 24:
                    sws_scale( getContext( &file->videoStream->conversionContext,
                                           file->videoStream->_ctx->width,
                                           file->videoStream->_ctx->height,
                                           file->videoStream->_ctx->pix_fmt,
                                           frame->surface->w, frame->surface->h,
                                           AV_PIX_FMT_RGB24 ),
                               ( const uint8_t* const* )file->videoStream->decodeFrame->data,
                               file->videoStream->decodeFrame->linesize,
                               0,
                               file->videoStream->_ctx->height,
                               ( uint8_t* const* )&frame->surface->pixels,
                               &pitch );
                    break;
                default:
                    break;
            }
        }

        /* we write the lastTimestamp we got */
        file->videoStream->lastTimeStamp = frame->pts;

        /* flag this frame as ready */
        frame->ready = 1;
    }

    return frame->ready;
}
/**
\endcond
*/
#endif
