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

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include <SDL.h>
#include <SDL_thread.h>

#ifdef __cplusplus
extern "C"
{
#endif
#include "libavformat/avformat.h"
#include "libavutil/mathematics.h"
#include "libswscale/swscale.h"
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

#ifdef __cplusplus
extern "C"
{
#endif

// Use audio conversion from Movie.cpp
extern int convert_audio(int in_samples, int in_channels, int in_stride,
                  enum AVSampleFormat in_fmt, const uint8_t *in_buf,
                  int out_samples, int out_channels, int out_stride,
                  enum AVSampleFormat out_fmt, uint8_t *out_buf);

#ifdef __cplusplus
}
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

uint32_t SDL_ffmpegInitWasCalled = 0;

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

const SDL_ffmpegCodec SDL_ffmpegCodecAUTO =
{
    -1,
    720, 576,
    1, 25,
    6000000,
    -1, -1,
    -1,
    2, 48000,
    192000,
    -1, -1
};

const SDL_ffmpegCodec SDL_ffmpegCodecPALDVD =
{
    AV_CODEC_ID_MPEG2VIDEO,
    720, 576,
    1, 25,
    6000000,
    -1, -1,
    AV_CODEC_ID_MP2,
    2, 48000,
    192000,
    -1, -1
};

const SDL_ffmpegCodec SDL_ffmpegCodecPALDV =
{
    AV_CODEC_ID_DVVIDEO,
    720, 576,
    1, 25,
    6553600,
    -1, -1,
    AV_CODEC_ID_DVAUDIO,
    2, 48000,
    256000,
    -1, -1
};

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


/** \brief  Initializes the SDL_ffmpeg library

            This is done automatically when using SDL_ffmpegOpen or
            SDL_ffmpegCreateFile. This means that it is usualy unnescecairy
            to explicitly call this function
*/
void SDL_ffmpegInit()
{
    /* register all codecs */
    if ( !SDL_ffmpegInitWasCalled )
    {
        SDL_ffmpegInitWasCalled = 1;

        avcodec_register_all();
        av_register_all();
    }
}

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

            av_free_packet( pack->data );

            av_free( pack->data );

            free( pack );
        }

        while ( old->conversionContext )
        {
            SDL_ffmpegConversionContext *ctx = old->conversionContext;

            old->conversionContext = old->conversionContext->next;

            sws_freeContext( ctx->context );

            free( ctx );
        }

        av_free( old->decodeFrame );

        if ( old->_ffmpeg ) avcodec_close( old->_ffmpeg->codec );

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

            av_free_packet( pack->data );

            av_free( pack->data );

            free( pack );
        }

        av_free( old->sampleBuffer );

        if ( old->_ffmpeg ) avcodec_close( old->_ffmpeg->codec );

        free( old );
    }

    if ( file->_ffmpeg )
    {
        if ( file->type == SDL_ffmpegInputStream )
        {
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(53,17,0)
            av_close_input_file( file->_ffmpeg );
#else
            avformat_close_input( &file->_ffmpeg );
#endif
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
    SDL_ffmpegInit();

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

        if ( file->_ffmpeg->streams[i]->codec->codec_type == AVMEDIA_TYPE_VIDEO )
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
                AVCodec *codec = avcodec_find_decoder( stream->_ffmpeg->codec->codec_id );

                if ( !codec )
                {
                    free( stream );
                    SDL_ffmpegSetError( "could not find video codec" );
                }
                else if ( avcodec_open2( file->_ffmpeg->streams[i]->codec, codec, NULL ) < 0 )
                {
                    free( stream );
                    SDL_ffmpegSetError( "could not open video codec" );
                }
                else
                {
                    stream->mutex = SDL_CreateMutex();

#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,0)
                    stream->decodeFrame = avcodec_alloc_frame();
#else
                    stream->decodeFrame = av_frame_alloc();
#endif

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
        else if ( file->_ffmpeg->streams[i]->codec->codec_type == AVMEDIA_TYPE_AUDIO )
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
                AVCodec *codec = avcodec_find_decoder( file->_ffmpeg->streams[i]->codec->codec_id );

                if ( !codec )
                {
                    free( stream );
                    SDL_ffmpegSetError( "could not find audio codec" );
                }
                else if ( avcodec_open2( file->_ffmpeg->streams[i]->codec, codec, NULL ) < 0 )
                {
                    free( stream );
                    SDL_ffmpegSetError( "could not open audio codec" );
                }
                else
                {
                    stream->mutex = SDL_CreateMutex();

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

#ifdef SDL_FF_WRITE
/** \brief  Use this to create the multimedia file of your choice.

            This function is used to create a multimedia file.

\param      filename string containing the location to which the data will be written
\returns    a pointer to a SDL_ffmpegFile structure, or NULL if a file could not be opened
*/
SDL_ffmpegFile* SDL_ffmpegCreate( const char* filename )
{
    SDL_ffmpegInit();

    SDL_ffmpegFile *file = SDL_ffmpegCreateFile();

    file->_ffmpeg = avformat_alloc_context();

    /* guess output format based on filename */
#if ( LIBAVFORMAT_VERSION_MAJOR <= 52 && LIBAVFORMAT_VERSION_MINOR <= 45 )
    file->_ffmpeg->oformat = guess_format( 0, filename, 0 );
#else
    file->_ffmpeg->oformat = av_guess_format( 0, filename, 0 );
#endif

    if ( !file->_ffmpeg->oformat )
    {
#if ( LIBAVFORMAT_VERSION_MAJOR <= 52 && LIBAVFORMAT_VERSION_MINOR <= 45 )
        file->_ffmpeg->oformat = guess_format( "dvd", 0, 0 );
#else
        file->_ffmpeg->oformat = av_guess_format( "dvd", 0, 0 );
#endif
    }

    /* preload as shown in ffmpeg.c */
    file->_ffmpeg->preload = ( int )( 0.5 * AV_TIME_BASE );

    /* max delay as shown in ffmpeg.c */
    file->_ffmpeg->max_delay = ( int )( 0.7 * AV_TIME_BASE );

    /* open the output file, if needed */
    if ( url_fopen( &file->_ffmpeg->pb, filename, AVIO_FLAG_WRITE ) < 0 )
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
#endif


#ifdef SDL_FF_WRITE
/** \brief  Use this to add a SDL_ffmpegVideoFrame to file

            By adding frames to file, a video stream is build. If an audio stream
            is present, syncing of both streams needs to be done by user.
\param      file SDL_ffmpegFile to which a frame needs to be added.
\param      frame SDL_ffmpegVideoFrame which will be added to the stream.
\returns    0 if frame was added, non-zero if an error occured.
*/
int SDL_ffmpegAddVideoFrame( SDL_ffmpegFile *file, SDL_Surface *frame )
{
    /* when accesing audio/video stream, streamMutex should be locked */
    SDL_LockMutex( file->streamMutex );

    if ( !file->videoStream || !frame || !frame->format )
    {
        SDL_UnlockMutex( file->streamMutex );
        return -1;
    }

    int pitch [] =
    {
        frame->pitch,
        0
    };

    const uint8_t *const data [] =
    {
        (uint8_t *)frame->pixels,
        0
    };

    switch ( frame->format->BitsPerPixel )
    {
        case 24:
            sws_scale( getContext( &file->videoStream->conversionContext,
                                   frame->w, frame->h, AV_PIX_FMT_RGB24,
                                   file->videoStream->_ffmpeg->codec->width,
                                   file->videoStream->_ffmpeg->codec->height,
                                   file->videoStream->_ffmpeg->codec->pix_fmt ),
                       data,
                       pitch,
                       0,
                       frame->h,
                       file->videoStream->encodeFrame->data,
                       file->videoStream->encodeFrame->linesize );
            break;
        case 32:
            sws_scale( getContext( &file->videoStream->conversionContext,
                                   frame->w, frame->h, AV_PIX_FMT_BGR32,
                                   file->videoStream->_ffmpeg->codec->width,
                                   file->videoStream->_ffmpeg->codec->height,
                                   file->videoStream->_ffmpeg->codec->pix_fmt ),
                       data,
                       pitch,
                       0,
                       frame->h,
                       file->videoStream->encodeFrame->data,
                       file->videoStream->encodeFrame->linesize );
            break;
        default:
            break;
    }

    /* PAL = upper field first
    file->videoStream->encodeFrame->top_field_first = 1;
    */

    int out_size = avcodec_encode_video( file->videoStream->_ffmpeg->codec, file->videoStream->encodeFrameBuffer, file->videoStream->encodeFrameBufferSize, file->videoStream->encodeFrame );

    /* if zero size, it means the image was buffered */
    if ( out_size > 0 )
    {
        AVPacket pkt;
        av_init_packet( &pkt );

        /* set correct stream index for this packet */
        pkt.stream_index = file->videoStream->_ffmpeg->index;
        /* set keyframe flag if needed */
        if ( file->videoStream->_ffmpeg->codec->coded_frame->key_frame ) pkt.flags |= AV_PKT_FLAG_KEY;
        /* write encoded data into packet */
        pkt.data = file->videoStream->encodeFrameBuffer;
        /* set the correct size of this packet */
        pkt.size = out_size;
        /* set the correct duration of this packet */
        pkt.duration = AV_TIME_BASE / file->videoStream->_ffmpeg->time_base.den;

        /* if needed info is available, write pts for this packet */
        if ( file->videoStream->_ffmpeg->codec->coded_frame->pts != AV_NOPTS_VALUE )
        {
            pkt.pts = av_rescale_q( file->videoStream->_ffmpeg->codec->coded_frame->pts, file->videoStream->_ffmpeg->codec->time_base, file->videoStream->_ffmpeg->time_base );
        }

        av_write_frame( file->_ffmpeg, &pkt );

        av_free_packet( &pkt );

        file->videoStream->frameCount++;
    }

    SDL_UnlockMutex( file->streamMutex );

    return 0;
}
#endif


#ifdef SDL_FF_WRITE
/** \brief  Use this to add a SDL_ffmpegAudioFrame to file

            By adding frames to file, an audio stream is build. If a video stream
            is present, syncing of both streams needs to be done by user.
\param      file SDL_ffmpegFile to which a frame needs to be added.
\param      frame SDL_ffmpegAudioFrame which will be added to the stream.
\returns    0 if frame was added, non-zero if an error occured.
*/
int SDL_ffmpegAddAudioFrame( SDL_ffmpegFile *file, SDL_ffmpegAudioFrame *frame )
{
    /* when accesing audio/video stream, streamMutex should be locked */
    SDL_LockMutex( file->streamMutex );

    if ( !file  || !file->audioStream || !frame )
    {
        SDL_UnlockMutex( file->streamMutex );
        return -1;
    }

    AVPacket pkt;

    /* initialize a packet to write */
    av_init_packet( &pkt );

    /* set correct stream index for this packet */
    pkt.stream_index = file->audioStream->_ffmpeg->index;

    /* set keyframe flag if needed */
    pkt.flags |= AV_PKT_FLAG_KEY;

    /* set the correct size of this packet */
    pkt.size = avcodec_encode_audio( file->audioStream->_ffmpeg->codec, ( uint8_t* )file->audioStream->sampleBuffer, file->audioStream->sampleBufferSize, ( int16_t* )frame->buffer );

    /* write encoded data into packet */
    pkt.data = ( uint8_t* )file->audioStream->sampleBuffer;

    /* if needed info is available, write pts for this packet */
    if ( file->audioStream->_ffmpeg->codec->coded_frame->pts != AV_NOPTS_VALUE )
    {
        pkt.pts = av_rescale_q( file->audioStream->_ffmpeg->codec->coded_frame->pts, file->audioStream->_ffmpeg->codec->time_base, file->audioStream->_ffmpeg->time_base );
    }

    /* write packet to stream */
    av_write_frame( file->_ffmpeg, &pkt );

    av_free_packet( &pkt );

    file->audioStream->frameCount++;

    SDL_UnlockMutex( file->streamMutex );

    return 0;
}
#endif

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
        bytes = file->audioStream->encodeAudioInputSize * 2 * file->audioStream->_ffmpeg->codec->channels;
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
        av_free_packet( pack->data );

        av_free( pack->data );

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

            av_free_packet( old->data );

            av_free( old->data );

            free( old );
        }

        file->audioStream->buffer = 0;

        /* flush internal ffmpeg buffers */
        if ( file->audioStream->_ffmpeg )
        {
            avcodec_flush_buffers( file->audioStream->_ffmpeg->codec );
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

            av_free_packet( old->data );

            av_free( old->data );

            free( old );
        }

        file->videoStream->buffer = 0;

        /* flush internal ffmpeg buffers */
        if ( file->videoStream->_ffmpeg ) avcodec_flush_buffers( file->videoStream->_ffmpeg->codec );

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
        av_free_packet( pack->data );

        av_free( pack->data );

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
    if ( stream && stream->_ffmpeg && stream->_ffmpeg->codec )
    {
        AVRational frate;
#if LIBAVFORMAT_VERSION_INT < AV_VERSION_INT(55,12,100)
        frate = stream->_ffmpeg->r_frame_rate;
#elif defined(av_stream_get_r_frame_rate)
        frate = av_stream_get_r_frame_rate(stream->_ffmpeg);
#else
        frate = stream->_ffmpeg->avg_frame_rate;
#endif
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
        spec.freq = file->audioStream->_ffmpeg->codec->sample_rate;
        spec.channels = ( uint8_t )file->audioStream->_ffmpeg->codec->channels;
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
            duration = file->audioStream->frameCount * file->audioStream->encodeAudioInputSize / ( file->audioStream->_ffmpeg->codec->sample_rate / 1000 );
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
            duration = av_rescale( 1000 * file->videoStream->frameCount, file->videoStream->_ffmpeg->codec->time_base.num, file->videoStream->_ffmpeg->codec->time_base.den );
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
        *w = file->videoStream->_ffmpeg->codec->width;
        *h = file->videoStream->_ffmpeg->codec->height;

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


#ifdef SDL_FF_WRITE
/** \brief  This is used to add a video stream to file

\param      file SDL_ffmpegFile to which the stream will be added
\param      codec SDL_ffmpegCodec describing the type encoding to be used by this
                  stream.
\returns    The stream which was added, or NULL if no stream could be added.
*/
SDL_ffmpegStream* SDL_ffmpegAddVideoStream( SDL_ffmpegFile *file, SDL_ffmpegCodec codec )
{
    /* add a video stream */
    AVStream *stream = av_new_stream( file->_ffmpeg, 0 );
    if ( !stream )
    {
        SDL_ffmpegSetError( "could not allocate video stream" );
        return 0;
    }

    stream->codec = avcodec_alloc_context();

    avcodec_get_context_defaults2( stream->codec, AVMEDIA_TYPE_VIDEO );

    if ( codec.videoCodecID < 0 )
    {
        stream->codec->codec_id = file->_ffmpeg->oformat->video_codec;
    }
    else
    {
        stream->codec->codec_id = ( enum CodecID ) codec.videoCodecID;
    }

    stream->codec->codec_type = AVMEDIA_TYPE_VIDEO;

    stream->codec->bit_rate = codec.videoBitrate;

    /* resolution must be a multiple of two */
    stream->codec->width = codec.width;
    stream->codec->height = codec.height;

    /* set time_base */
    stream->codec->time_base.num = codec.framerateNum;
    stream->codec->time_base.den = codec.framerateDen;

    /* emit one intra frame every twelve frames at most */
    stream->codec->gop_size = 12;

    /* set pixel format */
    stream->codec->pix_fmt = AV_PIX_FMT_YUV420P;

    /* set mpeg2 codec parameters */
    if ( stream->codec->codec_id == AV_CODEC_ID_MPEG2VIDEO )
    {
        stream->codec->max_b_frames = 2;
    }

    /* set mpeg1 codec parameters */
    if ( stream->codec->codec_id == AV_CODEC_ID_MPEG1VIDEO )
    {
        /* needed to avoid using macroblocks in which some coeffs overflow
           this doesnt happen with normal video, it just happens here as the
           motion of the chroma plane doesnt match the luma plane */
        stream->codec->mb_decision = 2;
    }

    /* some formats want stream headers to be separate */
    if ( file->_ffmpeg->oformat->flags & AVFMT_GLOBALHEADER )
    {
        stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }

    /* find the video encoder */
    AVCodec *videoCodec = avcodec_find_encoder( stream->codec->codec_id );
    if ( !videoCodec )
    {
        SDL_ffmpegSetError( "video codec not found" );
        return 0;
    }

    /* open the codec */
    if ( avcodec_open( stream->codec, videoCodec ) < 0 )
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

        str->id = file->audioStreams + file->videoStreams;

        /* _ffmpeg holds data about streamcodec */
        str->_ffmpeg = stream;

        str->mutex = SDL_CreateMutex();

        str->encodeFrame = avcodec_alloc_frame();

        uint8_t *picture_buf;
        int size = avpicture_get_size( stream->codec->pix_fmt, stream->codec->width, stream->codec->height );
        picture_buf = ( uint8_t* )av_malloc( size + FF_INPUT_BUFFER_PADDING_SIZE );
        avpicture_fill(( AVPicture* )str->encodeFrame, picture_buf, stream->codec->pix_fmt, stream->codec->width, stream->codec->height );

        str->encodeFrameBufferSize = stream->codec->width * stream->codec->height * 4 + FF_INPUT_BUFFER_PADDING_SIZE;

        str->encodeFrameBuffer = ( uint8_t* )av_malloc( str->encodeFrameBufferSize );

        file->videoStreams++;

        /* find correct place to save the stream */
        SDL_ffmpegStream **s = &file->vs;
        while ( *s )
        {
            *s = ( *s )->next;
        }

        *s = str;

        if ( av_set_parameters( file->_ffmpeg, 0 ) < 0 )
        {
            SDL_ffmpegSetError( "could not set encoding parameters" );
        }

        /* try to write a header */
        av_write_header( file->_ffmpeg );
    }

    return str;
}
#endif


#ifdef SDL_FF_WRITE
/** \brief  This is used to add a video stream to file

\param      file SDL_ffmpegFile to which the stream will be added
\param      codec SDL_ffmpegCodec describing the type encoding to be used by this
                  stream.
\returns    The stream which was added, or NULL if no stream could be added.
*/
SDL_ffmpegStream* SDL_ffmpegAddAudioStream( SDL_ffmpegFile *file, SDL_ffmpegCodec codec )
{
    // add an audio stream
    AVStream *stream = av_new_stream( file->_ffmpeg, 1 );
    if ( !stream )
    {
        SDL_ffmpegSetError( "could not allocate audio stream" );
        return 0;
    }

    if ( codec.audioCodecID < 0 )
    {
        stream->codec->codec_id = file->_ffmpeg->oformat->audio_codec;
    }
    else
    {
        stream->codec->codec_id = ( enum CodecID ) codec.audioCodecID;
    }

    stream->codec->codec_type = AVMEDIA_TYPE_AUDIO;
    stream->codec->bit_rate = codec.audioBitrate;
    stream->codec->sample_rate = codec.sampleRate;
    stream->codec->sample_fmt = AV_SAMPLE_FMT_S16;
    stream->codec->channels = codec.channels;

    // find the audio encoder
    AVCodec *audioCodec = avcodec_find_encoder( stream->codec->codec_id );
    if ( !audioCodec )
    {
        SDL_ffmpegSetError( "audio codec not found" );
        return 0;
    }

    // open the codec
    if ( avcodec_open( stream->codec, audioCodec ) < 0 )
    {
        SDL_ffmpegSetError( "could not open audio codec" );
        return 0;
    }

    /* some formats want stream headers to be separate */
    if ( file->_ffmpeg->oformat->flags & AVFMT_GLOBALHEADER )
    {
        stream->codec->flags |= CODEC_FLAG_GLOBAL_HEADER;
    }


    /* create a new stream */
    SDL_ffmpegStream *str = ( SDL_ffmpegStream* )malloc( sizeof( SDL_ffmpegStream ) );

    if ( str )
    {
        str->_ffmpeg = stream;

        /* we set our stream to zero */
        memset( str, 0, sizeof( SDL_ffmpegStream ) );

        str->id = file->audioStreams + file->videoStreams;

        /* _ffmpeg holds data about streamcodec */
        str->_ffmpeg = stream;

        str->mutex = SDL_CreateMutex();

        str->sampleBufferSize = 10000;

        str->sampleBuffer = ( int8_t* )av_malloc( str->sampleBufferSize );

        /* ugly hack for PCM codecs (will be removed ASAP with new PCM
           support to compute the input frame size in samples */
        if ( stream->codec->frame_size <= 1 )
        {
            str->encodeAudioInputSize = str->sampleBufferSize / stream->codec->channels;

            switch ( stream->codec->codec_id )
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
            str->encodeAudioInputSize = stream->codec->frame_size;
        }

        file->audioStreams++;

        /* find correct place to save the stream */
        SDL_ffmpegStream **s = &file->as;
        while ( *s )
        {
            *s = ( *s )->next;
        }

        *s = str;

        if ( av_set_parameters( file->_ffmpeg, 0 ) < 0 )
        {
            SDL_ffmpegSetError( "could not set encoding parameters" );
            return 0;
        }

        /* try to write a header */
        av_write_header( file->_ffmpeg );
    }

    return str;
}
#endif


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
    AVPacket *pack = ( AVPacket* )av_malloc( sizeof( AVPacket ) );

    /* initialize packet */
    av_init_packet( pack );

    /* read a packet from the file */
    int decode = av_read_frame( file->_ffmpeg, pack );

    /* if we did not get a packet, we probably reached the end of the file */
    if ( decode < 0 )
    {
        av_free( pack );

        /* signal EOF */
        return 1;
    }

    /* we got a packet, lets handle it */

    /* try to allocate the packet */
    if ( av_dup_packet( pack ) )
    {
        /* error allocating packet */
        av_free_packet( pack );
    }
    else
    {
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
            av_free_packet( pack );
        }
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
    uint8_t *data = pack->data;
    int size = pack->size;
    int audioSize = AVCODEC_MAX_AUDIO_FRAME_SIZE * sizeof( int16_t );

    int channels = file->audioStream->_ffmpeg->codec->channels;
    enum AVSampleFormat in_fmt = file->audioStream->_ffmpeg->codec->sample_fmt;
    int in_bps = av_get_bytes_per_sample(in_fmt);
    enum AVSampleFormat out_fmt = AV_SAMPLE_FMT_S16;
    int out_bps = av_get_bytes_per_sample(out_fmt);

    /* check if there is still data in the buffer */
    if ( file->audioStream->sampleBufferSize )
    {
        /* set new pts */
        if ( !frame->size ) frame->pts = file->audioStream->sampleBufferTime;

        /* calculate free space in frame */
        int fs = frame->capacity - frame->size;

        /* check the amount of data which needs to be copied */
        int in_samples = file->audioStream->sampleBufferSize / (channels * in_bps);
        int out_samples = fs / (channels * out_bps);
        
        if (out_samples < in_samples)
        {
            /* copy data from sampleBuffer into frame buffer until frame buffer is full */
            int written = convert_audio(out_samples, channels, file->audioStream->sampleBufferStride,
                                        in_fmt, (uint8_t *)(file->audioStream->sampleBuffer + file->audioStream->sampleBufferOffset),
                                        out_samples, channels, -1,
                                        out_fmt, frame->buffer + frame->size);
            
            /* mark the amount of bytes still in the buffer */
            file->audioStream->sampleBufferSize -= out_samples * channels * in_bps;

            /* move offset accordingly */
            if (av_sample_fmt_is_planar(in_fmt))
                file->audioStream->sampleBufferOffset += out_samples * in_bps;
            else
                file->audioStream->sampleBufferOffset += out_samples * in_bps * channels;

            /* update framesize */
            frame->size += written;
        }
        else
        {
            /* copy data from sampleBuffer into frame buffer until sampleBuffer is empty */
            int written = convert_audio(in_samples, channels, file->audioStream->sampleBufferStride,
                                        in_fmt, (uint8_t *)(file->audioStream->sampleBuffer + file->audioStream->sampleBufferOffset),
                                        in_samples, channels, -1,
                                        out_fmt, frame->buffer + frame->size);

            /* update framesize */
            frame->size += written;

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

    while ( size > 0 )
    {
        /* Decode the packet */
        AVCodecContext *avctx = file->audioStream->_ffmpeg->codec;
#if LIBAVCODEC_VERSION_INT < AV_VERSION_INT(55,28,1)
		AVFrame *dframe = avcodec_alloc_frame();
        avcodec_get_frame_defaults(dframe);
#else
		AVFrame *dframe = av_frame_alloc();
#endif
        int got_frame = 0;
        int len = avcodec_decode_audio4( avctx, dframe, &got_frame, pack );
        
        if (len < 0 || !got_frame)
        {
            SDL_ffmpegSetError( "error decoding audio frame" );
            break;
        }
        
        int planar = av_sample_fmt_is_planar( avctx->sample_fmt );
        int plane_size;
        int data_size = av_samples_get_buffer_size( &plane_size, avctx->channels, dframe->nb_samples, avctx->sample_fmt, 1 );
        if ( data_size > 10000 )
        {
            SDL_ffmpegSetError( "too much data in decoded audio frame" );
            break;
        }
        memcpy( file->audioStream->sampleBuffer, dframe->extended_data[0], plane_size );
        audioSize = plane_size;
        if ( planar && avctx->channels > 1 )
        {
            int8_t *out = file->audioStream->sampleBuffer + plane_size;
            int ch;
            for ( ch = 1; ch < avctx->channels; ch++ )
            {
                memcpy( out, dframe->extended_data[ch], plane_size );
                out += plane_size;
                audioSize += plane_size;
            }
        }

        /* change pointers */
        data += len;
        size -= len;
    }

    {
        /* set new pts */
        if ( !frame->size ) frame->pts = file->audioStream->sampleBufferTime;

        /* save stride of data we just grabbed */
        file->audioStream->sampleBufferStride = audioSize / channels;
        
        /* room in frame */
        int fs = frame->capacity - frame->size;

        /* check if there is room at all */
        if ( fs )
        {
            /* check the amount of data which needs to be copied */
            int in_samples = audioSize / (channels * in_bps);
            int out_samples = fs / (channels * out_bps);
            
            if (out_samples < in_samples)
            {
                /* copy data from sampleBuffer into frame buffer until frame buffer is full */
                int written = convert_audio(out_samples, channels, file->audioStream->sampleBufferStride,
                                            in_fmt, (uint8_t *)(file->audioStream->sampleBuffer),
                                            out_samples, channels, -1,
                                            out_fmt, frame->buffer + frame->size);

                /* mark the amount of bytes still in the buffer */
                file->audioStream->sampleBufferSize = ((in_samples - out_samples) * channels * in_bps);

                /* set the offset so the remaining data can be found */
                if (av_sample_fmt_is_planar(in_fmt))
                    file->audioStream->sampleBufferOffset = out_samples * in_bps;
                else
                    file->audioStream->sampleBufferOffset = out_samples * in_bps * channels;

                /* update framesize */
                frame->size += written;
            }
            else
            {
                /* copy data from sampleBuffer into frame buffer until sampleBuffer is empty */
                int written = convert_audio(in_samples, channels, file->audioStream->sampleBufferStride,
                                            in_fmt, (uint8_t *)(file->audioStream->sampleBuffer),
                                            in_samples, channels, -1,
                                            out_fmt, frame->buffer + frame->size);

                /* mark the amount of bytes still in the buffer */
                file->audioStream->sampleBufferSize = 0;

                /* reset buffer offset */
                file->audioStream->sampleBufferOffset = 0;

                /* update framesize */
                frame->size += written;
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
#if ( ( LIBAVCODEC_VERSION_MAJOR <= 52 ) && ( LIBAVCODEC_VERSION_MINOR <= 20 ) )
        avcodec_decode_video( file->videoStream->_ffmpeg->codec, file->videoStream->decodeFrame, &got_frame, pack->data, pack->size );
#else
        avcodec_decode_video2( file->videoStream->_ffmpeg->codec, file->videoStream->decodeFrame, &got_frame, pack );
#endif
    }
    else
    {
        /* check if there is still a frame left in the buffer */

#if ( LIBAVCODEC_VERSION_MAJOR <= 52 && LIBAVCODEC_VERSION_MINOR <= 20 )
        avcodec_decode_video( file->videoStream->_ffmpeg->codec, file->videoStream->decodeFrame, &got_frame, 0, 0 );
#else
        AVPacket temp;
        av_init_packet( &temp );
        temp.data = 0;
        temp.size = 0;
        temp.stream_index = file->videoStream->_ffmpeg->index;
        avcodec_decode_video2( file->videoStream->_ffmpeg->codec, file->videoStream->decodeFrame, &got_frame, &temp );
#endif
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
                                           file->videoStream->_ffmpeg->codec->width,
                                           file->videoStream->_ffmpeg->codec->height,
                                           file->videoStream->_ffmpeg->codec->pix_fmt,
                                           frame->surface->w, frame->surface->h,
                                           AV_PIX_FMT_RGB32 ),
                               ( const uint8_t* const* )file->videoStream->decodeFrame->data,
                               file->videoStream->decodeFrame->linesize,
                               0,
                               file->videoStream->_ffmpeg->codec->height,
                               ( uint8_t* const* )&frame->surface->pixels,
                               &pitch );
                    break;
                case 24:
                    sws_scale( getContext( &file->videoStream->conversionContext,
                                           file->videoStream->_ffmpeg->codec->width,
                                           file->videoStream->_ffmpeg->codec->height,
                                           file->videoStream->_ffmpeg->codec->pix_fmt,
                                           frame->surface->w, frame->surface->h,
                                           AV_PIX_FMT_RGB24 ),
                               ( const uint8_t* const* )file->videoStream->decodeFrame->data,
                               file->videoStream->decodeFrame->linesize,
                               0,
                               file->videoStream->_ffmpeg->codec->height,
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
