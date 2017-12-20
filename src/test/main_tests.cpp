// Copyright (c) 2014-2016 The Bitcoin Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"
#include "validation.h"
#include "net.h"

#include "test/test_bitcoin.h"

#include <boost/signals2/signal.hpp>
#include <boost/test/unit_test.hpp>

BOOST_FIXTURE_TEST_SUITE(main_tests, TestingSetup)

static void TestBlockSubsidyHalvings(const Consensus::Params& consensusParams)
{
    int maxHalvings = 64;
    CAmount nInitialSubsidy = 50 * COIN;
    CAmount nPreviousSubsidy = nInitialSubsidy * 2; // for height == 0

    CAmount nBTNInitialSubsidy = (uint32_t)consensusParams.BTNSubsidy * COIN;
    CAmount nBTNPreviousSubsidy = nBTNInitialSubsidy * 2; // for height == BTNHeight

    BOOST_CHECK_EQUAL(nPreviousSubsidy, nInitialSubsidy * 2);
    for (int nHalvings = 0; nHalvings < maxHalvings; nHalvings++) {
        int nHeight = nHalvings * consensusParams.nSubsidyHalvingInterval;
        CAmount nSubsidy = GetBlockSubsidy(nHeight, consensusParams);
        if(nHeight < (uint32_t)consensusParams.BTNHeight){
            BOOST_CHECK(nSubsidy <= nInitialSubsidy);
            BOOST_CHECK_EQUAL(nSubsidy, nPreviousSubsidy / 2);
            nPreviousSubsidy = nSubsidy;

        }else if((uint32_t)consensusParams.BTNHeight <= nHeight && nHeight < (uint32_t)(consensusParams.BTNHeight + consensusParams.BTNPremineWindow)){
            BOOST_CHECK_EQUAL(nSubsidy, (uint32_t)consensusParams.BTNPremineSubsidy* COIN);
        }else{
            if(nHeight%consensusParams.nBTNSubsidyHalvingInterval==0){
            BOOST_CHECK(nSubsidy <= nBTNInitialSubsidy);
            BOOST_CHECK_EQUAL(nSubsidy, nBTNPreviousSubsidy / 2);
            nBTNPreviousSubsidy = nSubsidy;
            }
        }

    }
    BOOST_CHECK_EQUAL(GetBlockSubsidy(maxHalvings * consensusParams.nBTNSubsidyHalvingInterval, consensusParams), 0);
}

static void TestBlockSubsidyHalvings(int nSubsidyHalvingInterval)
{
    Consensus::Params consensusParams;
    consensusParams.nSubsidyHalvingInterval = nSubsidyHalvingInterval;
    TestBlockSubsidyHalvings(consensusParams);
}

BOOST_AUTO_TEST_CASE(block_subsidy_test)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    TestBlockSubsidyHalvings(chainParams->GetConsensus()); // As in main
    //TestBlockSubsidyHalvings(150); // As in regtest
    //TestBlockSubsidyHalvings(1000); // Just another interval
}

BOOST_AUTO_TEST_CASE(subsidy_limit_test)
{
    const auto chainParams = CreateChainParams(CBaseChainParams::MAIN);
    CAmount nSum = 0;
    for (int nHeight = 0; nHeight < 20000000; nHeight++) {
        CAmount nSubsidy = GetBlockSubsidy(nHeight, chainParams->GetConsensus());
        //BOOST_CHECK(nSubsidy <= 50 * COIN);
        nSum += nSubsidy;
        BOOST_CHECK(MoneyRange(nSum));
    }
    BOOST_CHECK_EQUAL(nSum, 211424249992860000ULL);
}

bool ReturnFalse() { return false; }
bool ReturnTrue() { return true; }

BOOST_AUTO_TEST_CASE(test_combiner_all)
{
    boost::signals2::signal<bool (), CombinerAll> Test;
    BOOST_CHECK(Test());
    Test.connect(&ReturnFalse);
    BOOST_CHECK(!Test());
    Test.connect(&ReturnTrue);
    BOOST_CHECK(!Test());
    Test.disconnect(&ReturnFalse);
    BOOST_CHECK(Test());
    Test.disconnect(&ReturnTrue);
    BOOST_CHECK(Test());
}
BOOST_AUTO_TEST_SUITE_END()
