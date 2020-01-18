/**
 * Copyright (C) 2011-2019 Aratelia Limited - Juan A. Rubio
 *
 * This file is part of Tizonia
 *
 * Tizonia is free software: you can redistribute it and/or modify it under the
 * terms of the GNU Lesser General Public License as published by the Free
 * Software Foundation, either version 3 of the License, or (at your option)
 * any later version.
 *
 * Tizonia is distributed in the hope that it will be useful, but WITHOUT ANY
 * WARRANTY; without even the implied warranty of MERCHANTABILITY or FITNESS
 * FOR A PARTICULAR PURPOSE.  See the GNU Lesser General Public License for
 * more details.
 *
 * You should have received a copy of the GNU Lesser General Public License
 * along with Tizonia.  If not, see <http://www.gnu.org/licenses/>.
 */

/**
 * @file   tiztuneingraphops.cpp
 * @author Juan A. Rubio <juan.rubio@aratelia.com>
 *
 * @brief  Tunein client graph actions / operations implementation
 *
 */

#ifdef HAVE_CONFIG_H
#include <config.h>
#endif

#include <algorithm>

#include <boost/bind.hpp>
#include <boost/make_shared.hpp>

#include <OMX_Component.h>
#include <OMX_Core.h>
#include <OMX_TizoniaExt.h>
#include <tizplatform.h>

#include "tiztuneinconfig.hpp"
#include "tiztuneingraphops.hpp"
#include "tizgraph.hpp"
#include "tizgraphutil.hpp"
#include "tizprobe.hpp"

#ifdef TIZ_LOG_CATEGORY_NAME
#undef TIZ_LOG_CATEGORY_NAME
#define TIZ_LOG_CATEGORY_NAME "tiz.play.graph.tunein.ops"
#endif

namespace graph = tiz::graph;

//
// tuneinops
//
graph::tuneinops::tuneinops (graph *p_graph,
                             const omx_comp_name_lst_t &comp_lst,
                             const omx_comp_role_lst_t &role_lst)
  : tiz::graph::ops (p_graph, comp_lst, role_lst),
    encoding_ (OMX_AUDIO_CodingAutoDetect),
    inital_graph_load_ (false)
{
  TIZ_INIT_OMX_PORT_STRUCT (renderer_pcmtype_, 0);
  TIZ_INIT_OMX_PORT_STRUCT (decoder_mp3type_, 0);
}

void graph::tuneinops::do_enable_auto_detection (const int handle_id,
                                                 const int port_id)
{
  tiztuneinconfig_ptr_t tunein_config
      = boost::dynamic_pointer_cast< tuneinconfig > (config_);
  assert (tunein_config);
  tiz::graph::ops::do_enable_auto_detection (handle_id, port_id);
}

void graph::tuneinops::do_disable_comp_ports (const int comp_id,
                                              const int port_id)
{
  OMX_U32 tunein_source_port = port_id;
  G_OPS_BAIL_IF_ERROR (
      util::disable_port (handles_[comp_id], tunein_source_port),
      "Unable to disable tunein source's output port.");
  clear_expected_port_transitions ();
  add_expected_port_transition (handles_[comp_id], tunein_source_port,
                                OMX_CommandPortDisable);
}

void graph::tuneinops::do_configure_comp (const int comp_id)
{
  if (comp_id == 0)
  {
    tiztuneinconfig_ptr_t tunein_config
        = boost::dynamic_pointer_cast< tuneinconfig > (config_);
    assert (tunein_config);

    G_OPS_BAIL_IF_ERROR (
        tiz::graph::util::set_tunein_api_key (handles_[0],
                                              tunein_config->get_api_key ()),
        "Unable to set OMX_TizoniaIndexParamAudioTuneinSession");

    G_OPS_BAIL_IF_ERROR (
        tiz::graph::util::set_tunein_playlist (
            handles_[0], playlist_->get_uri_list (),
            tunein_config->get_playlist_type (),
            tunein_config->get_search_type (),
            playlist_->shuffle ()),
        "Unable to set OMX_TizoniaIndexParamAudioTuneinPlaylist");

    const OMX_U32 port_id = 0;
    G_OPS_BAIL_IF_ERROR (
        tiz::graph::util::set_streaming_buffer_params (
            handles_[0], port_id, config_->get_buffer_seconds (), 0, 100),
        "Unable to set OMX_TizoniaIndexParamStreamingBuffer");
  }
}

void graph::tuneinops::do_load ()
{
  assert (!comp_lst_.empty ());
  assert (!role_lst_.empty ());

  // At this point we are going to instantiate the remaining component in the
  // graph, the audio decoder and the pcm renderer. The tunein source is already
  // instantiated and in
  // Executing state.

  assert (comp_lst_.size () == 1);
  assert (handles_.size () == 1);

  G_OPS_BAIL_IF_ERROR (
      get_encoding_type_from_tunein_source (),
      "Unable to retrieve the audio encoding from the tunein source.");

  omx_comp_name_lst_t comp_list;
  omx_comp_role_lst_t role_list;
  G_OPS_BAIL_IF_ERROR (add_decoder_to_component_list (comp_list, role_list),
                       "Unknown/unhandled stream format.");

  comp_list.push_back (tiz::graph::util::get_default_pcm_renderer ());
  role_list.push_back ("audio_renderer.pcm");

  tiz::graph::cbackhandler &cbacks = get_cback_handler ();
  G_OPS_BAIL_IF_ERROR (
      util::instantiate_comp_list (comp_list, handles_, h2n_, &(cbacks),
                                   cbacks.get_omx_cbacks ()),
      "Unable to instantiate the component list.");

  // Now add the new components to the base class lists
  comp_lst_.insert (comp_lst_.begin (), comp_list.begin (), comp_list.end ());
  role_lst_.insert (role_lst_.begin (), role_list.begin (), role_list.end ());

  if (inital_graph_load_)
  {
    inital_graph_load_ = false;
    tiztuneinconfig_ptr_t tunein_config
        = boost::dynamic_pointer_cast< tuneinconfig > (config_);
    assert (tunein_config);
    tiz::graph::util::dump_graph_info ("Tunein", "Connecting",
                                       tunein_config->get_api_key ().c_str ());
  }
}

void graph::tuneinops::do_configure ()
{
  if (last_op_succeeded ())
  {
    G_OPS_BAIL_IF_ERROR (override_decoder_and_renderer_sampling_rates (),
                         "Unable to override decoder/renderer sampling rates");
    G_OPS_BAIL_IF_ERROR (apply_pcm_codec_info_from_decoder (),
                         "Unable to set OMX_IndexParamAudioPcm");
    const std::string coding_type_str ("Tunein");
    tiz::graph::util::dump_graph_info (coding_type_str.c_str (), "Connected",
                                       playlist_->get_current_uri ().c_str ());
  }
}

void graph::tuneinops::do_loaded2idle ()
{
  if (last_op_succeeded ())
  {
    // Transition the decoder and the renderer components to Idle
    omx_comp_handle_lst_t decoder_and_renderer_handles;
    decoder_and_renderer_handles.push_back (handles_[1]);  // the decoder
    decoder_and_renderer_handles.push_back (handles_[2]);  // the renderer
    G_OPS_BAIL_IF_ERROR (
        util::transition_all (decoder_and_renderer_handles, OMX_StateIdle,
                              OMX_StateLoaded),
        "Unable to transition deoder and renderer from Loaded->Idle");
    clear_expected_transitions ();
    add_expected_transition (handles_[1], OMX_StateIdle);
    add_expected_transition (handles_[2], OMX_StateIdle);
  }
}

void graph::tuneinops::do_idle2exe ()
{
  if (last_op_succeeded ())
  {
    // Transition the decoder and the renderer components to Exe
    omx_comp_handle_lst_t decoder_and_renderer_handles;
    decoder_and_renderer_handles.push_back (handles_[1]);  // the decoder
    decoder_and_renderer_handles.push_back (handles_[2]);  // the renderer
    G_OPS_BAIL_IF_ERROR (
        util::transition_all (decoder_and_renderer_handles, OMX_StateExecuting,
                              OMX_StateIdle),
        "Unable to transition decoder and renderer from Idle->Exe");
    clear_expected_transitions ();
    add_expected_transition (handles_[1], OMX_StateExecuting);
    add_expected_transition (handles_[2], OMX_StateExecuting);
  }
}

void graph::tuneinops::do_reconfigure_tunnel (const int tunnel_id)
{
  if (last_op_succeeded ())
  {
    if (0 == tunnel_id)
    {
      do_reconfigure_first_tunnel ();
    }
    else if (1 == tunnel_id)
    {
      do_reconfigure_second_tunnel ();
    }
    else
    {
      assert (0);
    }
  }
}

void graph::tuneinops::do_skip ()
{
  if (last_op_succeeded () && 0 != jump_)
  {
    assert (!handles_.empty ());
    G_OPS_BAIL_IF_ERROR (util::apply_playlist_jump (handles_[0], jump_),
                         "Unable to skip in playlist");
    // Reset the jump value, to its default value
    jump_ = SKIP_DEFAULT_VALUE;
  }
}

void graph::tuneinops::do_retrieve_metadata ()
{
  OMX_U32 index = 0;
  const int tunein_index = 0;
  // Extract metadata from the tunein source
  while (OMX_ErrorNone == dump_metadata_item (index++, tunein_index))
  {
  };

  // Now extract metadata from the decoder
  const int decoder_index = 1;
  index = 0;
  const bool use_first_as_heading = false;
  while (OMX_ErrorNone
         == dump_metadata_item (index++, decoder_index, use_first_as_heading))
  {
  };

  OMX_GetParameter (handles_[2], OMX_IndexParamAudioPcm, &renderer_pcmtype_);

  // Now print renderer metadata
  printf ("     ");
  TIZ_PRINTF_C05 (
      "%ld Ch, %g KHz, %lu:%s:%s", renderer_pcmtype_.nChannels,
      ((float)renderer_pcmtype_.nSamplingRate) / 1000,
      renderer_pcmtype_.nBitPerSample,
      renderer_pcmtype_.eNumData == OMX_NumericalDataSigned ? "s" : "u",
      renderer_pcmtype_.eEndian == OMX_EndianBig ? "b" : "l");
  printf ("\n");
}

// TODO: Move this implementation to the base class (and remove also from
// httpservops)
OMX_ERRORTYPE
graph::tuneinops::switch_tunnel (const int tunnel_id,
                                 const OMX_COMMANDTYPE to_disabled_or_enabled)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;

  assert (0 == tunnel_id || 1 == tunnel_id);
  assert (to_disabled_or_enabled == OMX_CommandPortDisable
          || to_disabled_or_enabled == OMX_CommandPortEnable);

  if (to_disabled_or_enabled == OMX_CommandPortDisable)
  {
    rc = tiz::graph::util::disable_tunnel (handles_, tunnel_id);
  }
  else
  {
    rc = tiz::graph::util::enable_tunnel (handles_, tunnel_id);
  }

  if (OMX_ErrorNone == rc && 0 == tunnel_id)
  {
    const int tunein_source_index = 0;
    const int tunein_source_output_port = 0;
    add_expected_port_transition (handles_[tunein_source_index],
                                  tunein_source_output_port,
                                  to_disabled_or_enabled);
    const int decoder_index = 1;
    const int decoder_input_port = 0;
    add_expected_port_transition (handles_[decoder_index], decoder_input_port,
                                  to_disabled_or_enabled);
  }
  else if (OMX_ErrorNone == rc && 1 == tunnel_id)
  {
    const int decoder_index = 1;
    const int decoder_output_port = 1;
    add_expected_port_transition (handles_[decoder_index], decoder_output_port,
                                  to_disabled_or_enabled);
    const int renderer_index = 2;
    const int renderer_input_port = 0;
    add_expected_port_transition (handles_[renderer_index], renderer_input_port,
                                  to_disabled_or_enabled);
  }
  return rc;
}

OMX_ERRORTYPE
graph::tuneinops::add_decoder_to_component_list (omx_comp_name_lst_t &comp_list,
                                                 omx_comp_role_lst_t &role_list)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  switch (encoding_)
  {
    case OMX_AUDIO_CodingMP3:
    {
      comp_list.push_back ("OMX.Aratelia.audio_decoder.mp3");
      role_list.push_back ("audio_decoder.mp3");
    }
    break;
    case OMX_AUDIO_CodingAAC:
    {
      comp_list.push_back ("OMX.Aratelia.audio_decoder.aac");
      role_list.push_back ("audio_decoder.aac");
    }
    break;
    case OMX_AUDIO_CodingVORBIS:
    {
      comp_list.push_back ("OMX.Aratelia.audio_decoder.vorbis");
      role_list.push_back ("audio_decoder.vorbis");
    }
    break;
    default:
    {
      if (OMX_AUDIO_CodingOPUS == encoding_)
      {
        comp_list.push_back ("OMX.Aratelia.audio_decoder.opusfile.opus");
        role_list.push_back ("audio_decoder.opus");
      }
      else if (OMX_AUDIO_CodingFLAC == encoding_)
      {
        comp_list.push_back ("OMX.Aratelia.audio_decoder.flac");
        role_list.push_back ("audio_decoder.flac");
      }
      else
      {
        TIZ_LOG (
            TIZ_PRIORITY_ERROR,
            "[OMX_ErrorFormatNotDetected] : Unhandled encoding type [%d]...",
            encoding_);
        rc = OMX_ErrorFormatNotDetected;
      }
    }
    break;
  };
  return rc;
}

bool graph::tuneinops::probe_stream_hook ()
{
  return true;
}

OMX_ERRORTYPE graph::tuneinops::get_encoding_type_from_tunein_source ()
{
  OMX_PARAM_PORTDEFINITIONTYPE port_def;
  const OMX_U32 port_id = 0;
  TIZ_INIT_OMX_PORT_STRUCT (port_def, port_id);
  tiz_check_omx (
      OMX_GetParameter (handles_[0], OMX_IndexParamPortDefinition, &port_def));
  encoding_ = port_def.format.audio.eEncoding;
  return OMX_ErrorNone;
}

OMX_ERRORTYPE
graph::tuneinops::override_decoder_and_renderer_sampling_rates ()
{
  OMX_U32 channels = 2;
  OMX_U32 sampling_rate = 44100;
  std::string encoding_str;
  tiz_check_omx (get_channels_and_rate_from_decoder (channels, sampling_rate,
                                                     encoding_str));
  channels = 2;
  sampling_rate = 44100;
  tiz_check_omx (set_channels_and_rate_on_decoder (channels, sampling_rate));
  return set_channels_and_rate_on_renderer (channels, sampling_rate,
                                            encoding_str);
}

OMX_ERRORTYPE
graph::tuneinops::apply_pcm_codec_info_from_decoder ()
{
  OMX_U32 channels = 2;
  OMX_U32 sampling_rate = 44100;
  std::string encoding_str;

  tiz_check_omx (get_channels_and_rate_from_decoder (channels, sampling_rate,
                                                     encoding_str));
  tiz_check_omx (set_channels_and_rate_on_renderer (channels, sampling_rate,
                                                    encoding_str));
  return OMX_ErrorNone;
}

OMX_ERRORTYPE
graph::tuneinops::get_channels_and_rate_from_decoder (
    OMX_U32 &channels, OMX_U32 &sampling_rate, std::string &encoding_str) const
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  const OMX_HANDLETYPE handle = handles_[1];  // decoder's handle
  const OMX_U32 port_id = 1;                  // decoder's output port

  switch (encoding_)
  {
    case OMX_AUDIO_CodingMP3:
    {
      encoding_str = "mp3";
    }
    break;
    case OMX_AUDIO_CodingAAC:
    {
      encoding_str = "aac";
    }
    break;
    case OMX_AUDIO_CodingVORBIS:
    {
      encoding_str = "vorbis";
    }
    break;
    default:
    {
      if (OMX_AUDIO_CodingOPUS == encoding_)
      {
        encoding_str = "opus";
      }
      else if (OMX_AUDIO_CodingFLAC == encoding_)
      {
        encoding_str = "flac";
      }
      else
      {
        TIZ_LOG (
            TIZ_PRIORITY_ERROR,
            "[OMX_ErrorFormatNotDetected] : Unhandled encoding type [%d]...",
            encoding_);
        rc = OMX_ErrorFormatNotDetected;
      }
    }
    break;
  };

  if (OMX_ErrorNone == rc)
  {
    rc = tiz::graph::util::
        get_channels_and_rate_from_audio_port_v2< OMX_AUDIO_PARAM_PCMMODETYPE > (
            handle, port_id, OMX_IndexParamAudioPcm, channels, sampling_rate);
  }
  TIZ_LOG (TIZ_PRIORITY_TRACE, "outcome = [%s]", tiz_err_to_str (rc));

  return rc;
}

OMX_ERRORTYPE
graph::tuneinops::set_channels_and_rate_on_decoder (const OMX_U32 channels,
                                                    const OMX_U32 sampling_rate)
{
  OMX_ERRORTYPE rc = OMX_ErrorNone;
  const OMX_HANDLETYPE handle = handles_[1];  // decoder's handle
  const OMX_U32 port_id = 0;                  // decoder's input port

  TIZ_LOG (TIZ_PRIORITY_TRACE, "channels = [%d] sampling_rate = [%d]", channels,
           sampling_rate);

  switch (encoding_)
  {
    case OMX_AUDIO_CodingMP3:
    {
      tiz::graph::util::set_channels_and_rate_on_audio_port<
        OMX_AUDIO_PARAM_MP3TYPE > (handle, port_id, OMX_IndexParamAudioMp3,
                                   channels, sampling_rate);
    }
    break;
    case OMX_AUDIO_CodingAAC:
    {
      tiz::graph::util::set_channels_and_rate_on_audio_port<
        OMX_AUDIO_PARAM_AACPROFILETYPE > (handle, port_id, OMX_IndexParamAudioAac,
                                          channels, sampling_rate);
    }
    break;
    case OMX_AUDIO_CodingVORBIS:
    {
      // TODO
    }
    break;
    default:
    {
      if (OMX_AUDIO_CodingOPUS == encoding_)
      {
        // TODO
      }
      else if (OMX_AUDIO_CodingFLAC == encoding_)
      {
        // TODO
      }
      else
      {
        TIZ_LOG (
            TIZ_PRIORITY_ERROR,
            "[OMX_ErrorFormatNotDetected] : Unhandled encoding type [%d]...",
            encoding_);
        rc = OMX_ErrorFormatNotDetected;
      }
    }
    break;
  };

  return rc;
}

OMX_ERRORTYPE
graph::tuneinops::set_channels_and_rate_on_renderer (
    const OMX_U32 channels, const OMX_U32 sampling_rate,
    const std::string encoding_str)
{
  const OMX_HANDLETYPE handle = handles_[2];  // renderer's handle
  const OMX_U32 port_id = 0;                  // renderer's input port

  TIZ_LOG (TIZ_PRIORITY_TRACE, "channels = [%d] sampling_rate = [%d]", channels,
           sampling_rate);

  // Retrieve the pcm settings from the renderer component
  TIZ_INIT_OMX_PORT_STRUCT (renderer_pcmtype_, port_id);
  tiz_check_omx (
      OMX_GetParameter (handle, OMX_IndexParamAudioPcm, &renderer_pcmtype_));

  // Now assign the actual settings to the pcmtype structure
  renderer_pcmtype_.nChannels = channels;
  renderer_pcmtype_.nSamplingRate = sampling_rate;
  renderer_pcmtype_.eNumData = OMX_NumericalDataSigned;
  renderer_pcmtype_.eEndian
      = (encoding_ == OMX_AUDIO_CodingMP3 ? OMX_EndianBig : OMX_EndianLittle);

  if (OMX_AUDIO_CodingOPUS == encoding_ || OMX_AUDIO_CodingVORBIS == encoding_)
  {
    // Opus and Vorbis decoders output 32 bit samples (floats)
    renderer_pcmtype_.nBitPerSample = 32;
  }

  // Set the new pcm settings
  tiz_check_omx (
      OMX_SetParameter (handle, OMX_IndexParamAudioPcm, &renderer_pcmtype_));

  return OMX_ErrorNone;
}

bool graph::tuneinops::is_fatal_error (const OMX_ERRORTYPE error) const
{
  bool rc = false;
  TIZ_LOG (TIZ_PRIORITY_ERROR, "[%s] ", tiz_err_to_str (error));
  if (error == error_code_)
  {
    // if this error is already being handled, then ignore it.
    rc = false;
  }
  else
  {
    rc |= tiz::graph::ops::is_fatal_error (error);
    rc |= (OMX_ErrorContentURIError == error);
  }
  return rc;
}

void graph::tuneinops::do_record_fatal_error (const OMX_HANDLETYPE handle,
                                              const OMX_ERRORTYPE error,
                                              const OMX_U32 port,
                                              const OMX_PTR p_eventdata /* = NULL */)
{
  tiz::graph::ops::do_record_fatal_error (handle, error, port, p_eventdata);
  if (error == OMX_ErrorContentURIError)
  {
    error_msg_.append ("\n [Playlist not found]");
  }
}

void graph::tuneinops::do_reconfigure_first_tunnel ()
{
  switch (encoding_)
  {
    case OMX_AUDIO_CodingMP3:
    {
      // Retrieve the mp3 settings from the tunein source component
      OMX_AUDIO_PARAM_MP3TYPE tunein_mp3type;
      const OMX_U32 tunein_port_id = 0;
      TIZ_INIT_OMX_PORT_STRUCT (tunein_mp3type, tunein_port_id);
      G_OPS_BAIL_IF_ERROR (
          OMX_GetParameter (handles_[0], OMX_IndexParamAudioMp3,
                            &tunein_mp3type),
          "Unable to retrieve the MP3 settings from the tunein source");

      // Retrieve the mp3 settings from the decoder component
      OMX_AUDIO_PARAM_MP3TYPE decoder_mp3type;
      const OMX_U32 decoder_port_id = 0;
      TIZ_INIT_OMX_PORT_STRUCT (decoder_mp3type, decoder_port_id);
      G_OPS_BAIL_IF_ERROR (
          OMX_GetParameter (handles_[1], OMX_IndexParamAudioMp3,
                            &decoder_mp3type),
          "Unable to retrieve the MP3 settings from the audio decoder");

      // Now assign the current settings to the decoder structure
      decoder_mp3type.nChannels = tunein_mp3type.nChannels;
      decoder_mp3type.nSampleRate = tunein_mp3type.nSampleRate;

      // Set the new mp3 settings
      G_OPS_BAIL_IF_ERROR (
          OMX_SetParameter (handles_[1], OMX_IndexParamAudioMp3,
                            &decoder_mp3type),
          "Unable to set the MP3 settings on the audio decoder");
    }
    break;
    default:
    {
      // TODO
    }
    break;
  };
}

void graph::tuneinops::do_reconfigure_second_tunnel ()
{
  // Retrieve the pcm settings from the decoder component
  OMX_AUDIO_PARAM_PCMMODETYPE decoder_pcmtype;
  const OMX_U32 decoder_port_id = 1;
  TIZ_INIT_OMX_PORT_STRUCT (decoder_pcmtype, decoder_port_id);
  G_OPS_BAIL_IF_ERROR (
      OMX_GetParameter (handles_[1], OMX_IndexParamAudioPcm, &decoder_pcmtype),
      "Unable to retrieve the PCM settings from the decoder");

  // Retrieve the pcm settings from the renderer component
  OMX_AUDIO_PARAM_PCMMODETYPE renderer_pcmtype;
  const OMX_U32 renderer_port_id = 0;
  TIZ_INIT_OMX_PORT_STRUCT (renderer_pcmtype, renderer_port_id);
  G_OPS_BAIL_IF_ERROR (
      OMX_GetParameter (handles_[2], OMX_IndexParamAudioPcm, &renderer_pcmtype),
      "Unable to retrieve the PCM settings from the pcm renderer");

  // Now assign the current settings to the renderer structure
  renderer_pcmtype.nChannels = decoder_pcmtype.nChannels;
  renderer_pcmtype.nSamplingRate = decoder_pcmtype.nSamplingRate;

  // Set the new pcm settings
  G_OPS_BAIL_IF_ERROR (
      OMX_SetParameter (handles_[2], OMX_IndexParamAudioPcm, &renderer_pcmtype),
      "Unable to set the PCM settings on the audio renderer");

  printf ("     ");
  TIZ_PRINTF_C05 (
      "%ld Ch, %g KHz, %lu:%s:%s", renderer_pcmtype.nChannels,
      ((float)renderer_pcmtype.nSamplingRate) / 1000,
      renderer_pcmtype.nBitPerSample,
      renderer_pcmtype.eNumData == OMX_NumericalDataSigned ? "s" : "u",
      renderer_pcmtype.eEndian == OMX_EndianBig ? "b" : "l");
  printf ("\n");
}
