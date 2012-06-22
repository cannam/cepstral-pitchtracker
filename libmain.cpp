/* -*- c-basic-offset: 4 indent-tabs-mode: nil -*-  vi:set ts=8 sts=4 sw=4: */

#include <vamp/vamp.h>
#include <vamp-sdk/PluginAdapter.h>

#include "SimpleCepstrum.h"

static Vamp::PluginAdapter<SimpleCepstrum> cepPluginAdapter;

const VampPluginDescriptor *
vampGetPluginDescriptor(unsigned int version, unsigned int index)
{
    if (version < 1) return 0;

    switch (index) {
    case  0: return cepPluginAdapter.getDescriptor();
    default: return 0;
    }
}


