/*
    This file is part of Magnum.

    Copyright © 2010, 2011, 2012, 2013, 2014, 2015
              Vladimír Vondruš <mosra@centrum.cz>

    Permission is hereby granted, free of charge, to any person obtaining a
    copy of this software and associated documentation files (the "Software"),
    to deal in the Software without restriction, including without limitation
    the rights to use, copy, modify, merge, publish, distribute, sublicense,
    and/or sell copies of the Software, and to permit persons to whom the
    Software is furnished to do so, subject to the following conditions:

    The above copyright notice and this permission notice shall be included
    in all copies or substantial portions of the Software.

    THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
    IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
    FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
    THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
    LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
    FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER
    DEALINGS IN THE SOFTWARE.
*/

#include "OpenGexImporter.h"

#include <Corrade/Containers/Array.h>

#include "MagnumPlugins/OpenGexImporter/OpenDdl/Document.h"

#include "openGexSpec.hpp"

namespace Magnum { namespace Trade {

struct OpenGexImporter::Document {
    OpenDdl::Document document;

    /* Default metrics */
    Float distanceMultiplier = 1.0f;
    Float angleMultiplier = 1.0f;
    Float timeMultiplier = 1.0f;
    bool yUp = false;
};

OpenGexImporter::OpenGexImporter() = default;

OpenGexImporter::OpenGexImporter(PluginManager::Manager<AbstractImporter>& manager): AbstractImporter(manager) {}

OpenGexImporter::OpenGexImporter(PluginManager::AbstractManager& manager, std::string plugin): AbstractImporter(manager, plugin) {}

OpenGexImporter::~OpenGexImporter() = default;

auto OpenGexImporter::doFeatures() const -> Features { return Feature::OpenData; }

bool OpenGexImporter::doIsOpened() const { return !!_d; }

void OpenGexImporter::doOpenData(const Containers::ArrayReference<const char> data) {
    std::unique_ptr<Document> d{new Document};

    /* Parse the document */
    if(!d->document.parse(data, OpenGex::structures, OpenGex::properties)) return;

    /* Validate the document */
    if(!d->document.validate(OpenGex::rootStructures, OpenGex::structureInfo)) return;

    /* Metrics */
    for(const OpenDdl::Structure metric: d->document.childrenOf(OpenGex::Metric)) {
        auto&& key = metric.propertyOf(OpenGex::key).as<std::string>();
        const OpenDdl::Structure value = metric.firstChild();

        /* Distance multiplier */
        if(key == "distance") {
            if(value.type() != OpenDdl::Type::Float) {
                Error() << "Trade::OpenGexImporter::openData(): invalid value for distance metric";
                return;
            }

            d->distanceMultiplier = value.as<Float>();

        /* Angle multiplier */
        } else if(key == "angle") {
            if(value.type() != OpenDdl::Type::Float) {
                Error() << "Trade::OpenGexImporter::openData(): invalid value for angle metric";
                return;
            }

            d->angleMultiplier = value.as<Float>();

        /* Time multiplier */
        } else if(key == "time") {
            if(value.type() != OpenDdl::Type::Float) {
                Error() << "Trade::OpenGexImporter::openData(): invalid value for time metric";
                return;
            }

            d->timeMultiplier = value.as<Float>();

        /* Up axis */
        } else if(key == "up") {
            if(value.type() != OpenDdl::Type::String || (value.as<std::string>() != "y" && value.as<std::string>() != "z")) {
                Error() << "Trade::OpenGexImporter::openData(): invalid value for up metric";
                return;
            }

            d->yUp = value.as<std::string>() == "y";
        }
    }

    /* Everything okay, save the instance */
    _d = std::move(d);
}

void OpenGexImporter::doClose() { _d = nullptr; }

}}
