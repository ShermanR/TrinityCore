﻿/*
 * Copyright (C) 2012-2014 Arctium Emulation <http://arctium.org>
 * Copyright (C) 2008-2014 TrinityCore <http://www.trinitycore.org/>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * asize_t with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "Patcher.hpp"
#include "Helper.hpp"

#include <boost/filesystem.hpp>

#include <fstream>
#include <iostream>
#include <iterator>
#include <stdexcept>

namespace
{
    std::vector<unsigned char> read_file(boost::filesystem::path const& path)
    {
        std::ifstream ifs(path.string(), std::ifstream::binary);
        if (!ifs)
            throw std::runtime_error("could not open " + path.string());

        ifs >> std::noskipws;

        return {std::istream_iterator<unsigned char>(ifs), std::istream_iterator<unsigned char>()};
    }

    void write_file(boost::filesystem::path const& path, std::vector<unsigned char> const& data)
    {
        std::ofstream ofs(path.string(), std::ofstream::binary);
        if (!ofs)
            throw std::runtime_error("could not open " + path.string());

        ofs << std::noskipws;

        std::copy(data.begin(), data.end(), std::ostream_iterator<unsigned char>(ofs));
    }

    size_t SearchOffset (std::vector<unsigned char> const& binary, std::vector<unsigned char> const& pattern)
    {
        for (size_t i = 0; i < binary.size(); i++)
        {
            size_t matches = 0;

            for (size_t j = 0; j < pattern.size(); j++)
            {
                if (pattern.size() > (binary.size() - i))
                    throw std::runtime_error("unable to find pattern");

                if (pattern[j] == 0)
                {
                    matches++;
                    continue;
                }

                if (binary[i + j] != pattern[j])
                    break;

                matches++;
            }

            if (matches == pattern.size())
                return i;
        }

        throw std::runtime_error("unable to find pattern");
    }
}

namespace Connection_Patcher
{
    Patcher::Patcher(boost::filesystem::path file)
        : binary(read_file(file))
        , Type(Helper::GetBinaryType(binary))
    {}

    void Patcher::Patch(std::vector<unsigned char> const& bytes, std::vector<unsigned char> const& pattern)
    {
        if (binary.size() < pattern.size())
            throw std::logic_error("pattern larger than binary");

        if (pattern.empty())
            return;

        size_t const offset(SearchOffset(binary, pattern));
        std::cout << "Found offset " << offset << std::endl;

        if (offset != 0 && binary.size() >= bytes.size())
            for (size_t i = 0; i < bytes.size(); i++)
                binary[offset + i] = bytes[i];
    }

    void Patcher::Finish(boost::filesystem::path out)
    {
        if (boost::filesystem::exists(out))
            boost::filesystem::remove(out);

        write_file(out, binary);
    }
}