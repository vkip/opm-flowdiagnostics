/*
  Copyright 2016 SINTEF ICT, Applied Mathematics.
  Copyright 2016 Statoil ASA.

  This file is part of the Open Porous Media Project (OPM).

  OPM is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  OPM is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with OPM.  If not, see <http://www.gnu.org/licenses/>.
*/

#if HAVE_CONFIG_H
#include <config.h>
#endif // HAVE_CONFIG_H

#if HAVE_DYNAMIC_BOOST_TEST
#define BOOST_TEST_DYN_LINK
#endif

#define NVERBOSE

#define BOOST_TEST_MODULE TEST_ASSEMBLED_CONNECTIONS

#include <boost/test/unit_test.hpp>

#include <opm/flowdiagnostics/utility/AssembledConnections.hpp>

#include <exception>
#include <stdexcept>

namespace {

    template <class Collection1, class Collection2>
    void check_is_close(const Collection1& c1, const Collection2& c2)
    {
        BOOST_REQUIRE_EQUAL(c1.size(), c2.size());

        if (! c1.empty()) {
            auto i1 = c1.begin(), e1 = c1.end();
            auto i2 = c2.begin();

            for (; i1 != e1; ++i1, ++i2) {
                BOOST_CHECK_CLOSE(*i1, *i2, 1.0e-10);
            }
        }
    }

} // Namespace Anonymous

BOOST_AUTO_TEST_SUITE(Two_By_Two)

BOOST_AUTO_TEST_CASE (Constructor)
{
    auto g = Opm::Utility::AssembledConnections{};
}

BOOST_AUTO_TEST_CASE (Zero_To_One)
{
    auto g = Opm::Utility::AssembledConnections{};

    g.addConnection(0, 1);

    g.compress();

    {
        const auto start = g.startPointers();

        const auto expect_ia = std::vector<int>{ 0, 1 };

        BOOST_CHECK_EQUAL_COLLECTIONS(start    .begin(), start    .end(),
                                      expect_ia.begin(), expect_ia.end());
    }

    {
        const auto neigh = g.neighbourhood();

        const auto expect_ja = std::vector<int>{ 1 };

        BOOST_CHECK_EQUAL_COLLECTIONS(neigh    .begin(), neigh    .end(),
                                      expect_ja.begin(), expect_ja.end());
    }
}

BOOST_AUTO_TEST_CASE (Zero_To_One_Two)
{
    auto g = Opm::Utility::AssembledConnections{};

    g.addConnection(0, 2);
    g.addConnection(0, 1);

    g.compress();

    {
        const auto start = g.startPointers();

        const auto expect_ia = std::vector<int>{ 0, 2 };

        BOOST_CHECK_EQUAL_COLLECTIONS(start    .begin(), start    .end(),
                                      expect_ia.begin(), expect_ia.end());
    }

    {
        const auto neigh = g.neighbourhood();

        const auto expect_ja = std::vector<int>{ 1, 2 };

        BOOST_CHECK_EQUAL_COLLECTIONS(neigh    .begin(), neigh    .end(),
                                      expect_ja.begin(), expect_ja.end());
    }
}

BOOST_AUTO_TEST_CASE (Zero_To_One_Two_Duplicate)
{
    auto g = Opm::Utility::AssembledConnections{};

    for (auto i = 0, n = 3; i < n; ++i) {
        g.addConnection(0, 2);
    }

    g.addConnection(0, 1);

    for (auto i = 0, n = 3; i < n; ++i) {
        g.addConnection(0, 2);
    }

    for (auto i = 0, n = 3; i < n; ++i) {
        g.addConnection(0, 1);
    }

    g.compress();

    {
        const auto start = g.startPointers();

        const auto expect_ia = std::vector<int>{ 0, 2 };

        BOOST_CHECK_EQUAL_COLLECTIONS(start    .begin(), start    .end(),
                                      expect_ia.begin(), expect_ia.end());
    }

    {
        const auto neigh = g.neighbourhood();

        const auto expect_ja = std::vector<int>{ 1, 2 };

        BOOST_CHECK_EQUAL_COLLECTIONS(neigh    .begin(), neigh    .end(),
                                      expect_ja.begin(), expect_ja.end());
    }
}

BOOST_AUTO_TEST_CASE (Isolated_Node)
{
    auto g = Opm::Utility::AssembledConnections{};

    g.addConnection(2, 0);
    g.addConnection(0, 2);
    g.addConnection(0, 0);

    g.compress();

    {
        const auto start = g.startPointers();

        const auto expect_ia = std::vector<int>{ 0, 2, 2, 3 };

        BOOST_CHECK_EQUAL_COLLECTIONS(start    .begin(), start    .end(),
                                      expect_ia.begin(), expect_ia.end());
    }

    {
        const auto neigh = g.neighbourhood();

        const auto expect_ja = std::vector<int>{ 0, 2, 0 };

        BOOST_CHECK_EQUAL_COLLECTIONS(neigh    .begin(), neigh    .end(),
                                      expect_ja.begin(), expect_ja.end());
    }
}

BOOST_AUTO_TEST_CASE (All_To_All)
{
    auto g = Opm::Utility::AssembledConnections{};

    const auto n = 4;
    for (auto i = 0*n; i < n; ++i) {
        for (auto j = 0*n; j < n; ++j) {
            g.addConnection(j, i);
        }
    }

    g.compress();

    {
        const auto start = g.startPointers();

        const auto expect_ia = std::vector<int>{ 0, 4, 8, 12, 16 };

        BOOST_CHECK_EQUAL_COLLECTIONS(start    .begin(), start    .end(),
                                      expect_ia.begin(), expect_ia.end());
    }

    {
        const auto neigh = g.neighbourhood();

        const auto expect_ja = std::vector<int>{
            0, 1, 2, 3,
            0, 1, 2, 3,
            0, 1, 2, 3,
            0, 1, 2, 3,
        };

        BOOST_CHECK_EQUAL_COLLECTIONS(neigh    .begin(), neigh    .end(),
                                      expect_ja.begin(), expect_ja.end());
    }
}

BOOST_AUTO_TEST_CASE (All_To_All_Duplicate)
{
    auto g = Opm::Utility::AssembledConnections{};

    const auto n = 4;

    for (auto k = 0*n; k < n; ++k) {
        for (auto i = 0*n; i < n; ++i) {
            for (auto j = 0*n; j < n; ++j) {
                g.addConnection(i, i);
                g.addConnection(i, j);
                g.addConnection(j, i);
                g.addConnection(j, j);
            }
        }
    }

    g.compress();

    {
        const auto start = g.startPointers();

        const auto expect_ia = std::vector<int>{ 0, 4, 8, 12, 16 };

        BOOST_CHECK_EQUAL_COLLECTIONS(start    .begin(), start    .end(),
                                      expect_ia.begin(), expect_ia.end());
    }

    {
        const auto neigh = g.neighbourhood();

        const auto expect_ja = std::vector<int>{
            0, 1, 2, 3,
            0, 1, 2, 3,
            0, 1, 2, 3,
            0, 1, 2, 3,
        };

        BOOST_CHECK_EQUAL_COLLECTIONS(neigh    .begin(), neigh    .end(),
                                      expect_ja.begin(), expect_ja.end());
    }
}

BOOST_AUTO_TEST_CASE (Weighted_Graph_Single)
{
    auto g = Opm::Utility::AssembledConnections{};

    g.addConnection(0, 2,   0.2);
    g.addConnection(0, 1, - 0.1);

    g.addConnection(1, 3,  13.0);

    g.addConnection(2, 3, -23.0);

    g.compress();

    {
        const auto start = g.startPointers();

        const auto expect_ia = std::vector<int>{ 0, 2, 3, 4 };

        BOOST_CHECK_EQUAL_COLLECTIONS(start    .begin(), start    .end(),
                                      expect_ia.begin(), expect_ia.end());
    }

    {
        const auto neigh = g.neighbourhood();

        const auto expect_ja = std::vector<int>{ 1, 2, 3, 3 };

        BOOST_CHECK_EQUAL_COLLECTIONS(neigh    .begin(), neigh    .end(),
                                      expect_ja.begin(), expect_ja.end());
    }

    {
        const auto w = g.connectionWeight();

        const auto expect_w = std::vector<double>{
            - 0.1, 0.2,
             13.0,
            -23.0,
        };

        check_is_close(w, expect_w);
    }
}

BOOST_AUTO_TEST_CASE (Weighted_Graph_Multiple)
{
    auto g = Opm::Utility::AssembledConnections{};

    const auto count = 4;

    for (auto i = 0*count; i < count; ++i) {
        g.addConnection(0, 2,   0.2);
        g.addConnection(0, 1, - 0.1);

        g.addConnection(1, 3,  13.0);

        g.addConnection(2, 3, -23.0);
    }

    g.compress();

    {
        const auto start = g.startPointers();

        const auto expect_ia = std::vector<int>{ 0, 2, 3, 4 };

        BOOST_CHECK_EQUAL_COLLECTIONS(start    .begin(), start    .end(),
                                      expect_ia.begin(), expect_ia.end());
    }

    {
        const auto neigh = g.neighbourhood();

        const auto expect_ja = std::vector<int>{ 1, 2, 3, 3 };

        BOOST_CHECK_EQUAL_COLLECTIONS(neigh    .begin(), neigh    .end(),
                                      expect_ja.begin(), expect_ja.end());
    }

    {
        const auto w = g.connectionWeight();

        const auto expect_w = std::vector<double>{
            count * (- 0.1), count * 0.2,
            count *   13.0 ,
            count * (-23.0),
        };

        check_is_close(w, expect_w);
    }
}

BOOST_AUTO_TEST_CASE (Compress_Invalid_Throw)
{
    auto g = Opm::Utility::AssembledConnections{};

    g.addConnection(0, 1);
    g.addConnection(0, 2, 1.0);

    // Can't mix weighted and unweighted edges.
    BOOST_CHECK_THROW(g.compress(), std::logic_error);
}

BOOST_AUTO_TEST_SUITE_END()