/*
    This file is part of Element
    Copyright (C) 2014-2019  Kushview, LLC.  All rights reserved.

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <https://www.gnu.org/licenses/>.
*/

#ifndef ELEMENT_ASSET_H
#define ELEMENT_ASSET_H

#include "ElementApp.h"
#include "session/AssetTree.h"

namespace Element {

class AssetNode : public ObjectModel
{
public:
    AssetNode (const AssetItem& item);
    ~AssetNode();
};

} // namespace Element

#endif /* ELEMENT_ASSET_H */
