// Copyright (c) 2010 Satoshi Nakamoto
// Copyright (c) 2009-2015 The Bitcoin developers
// Copyright (c) 2014-2015 The Dash developers
// Copyright (c) 2015-2022 The PIVX Core developers
// Copyright (c) 2026 The 1776CASH Core developers
// Distributed under the MIT software license, see the accompanying
// file COPYING or http://www.opensource.org/licenses/mit-license.php.

#include "chainparams.h"

#include "chainparamsseeds.h"
#include "consensus/merkle.h"
#include "tinyformat.h"
#include "utilstrencodings.h"

#include <assert.h>

static CBlock CreateGenesisBlock(const char* pszTimestamp, const CScript& genesisOutputScript, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    CMutableTransaction txNew;
    txNew.nVersion = 1;
    txNew.vin.resize(1);
    txNew.vout.resize(1);
    txNew.vin[0].scriptSig = CScript() << 486604799 << CScriptNum(4) << std::vector<unsigned char>((const unsigned char*)pszTimestamp, (const unsigned char*)pszTimestamp + strlen(pszTimestamp));
    txNew.vout[0].nValue = genesisReward;
    txNew.vout[0].scriptPubKey = genesisOutputScript;

    CBlock genesis;
    genesis.vtx.push_back(std::make_shared<const CTransaction>(std::move(txNew)));
    genesis.hashPrevBlock.SetNull();
    genesis.nVersion = nVersion;
    genesis.nTime    = nTime;
    genesis.nBits    = nBits;
    genesis.nNonce   = nNonce;
    genesis.hashMerkleRoot = BlockMerkleRoot(genesis);
    return genesis;
}

void CChainParams::UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
{
    assert(idx > Consensus::BASE_NETWORK && idx < Consensus::MAX_NETWORK_UPGRADES);
    consensus.vUpgrades[idx].nActivationHeight = nActivationHeight;
}

namespace {
static constexpr int64_t GOVERNANCE_CYCLE_SECONDS = 14 * 24 * 60 * 60;
// 1776CASH targets a 55,555 monthly treasury while retaining the existing two 14-day governance cycles.
static constexpr CAmount MONTHLY_GOVERNANCE_BUDGET = 55555 * COIN;
static constexpr CAmount GOVERNANCE_CYCLE_BUDGET = MONTHLY_GOVERNANCE_BUDGET / 2;
static constexpr int GOVERNANCE_MAX_CYCLE_PAYMENTS = 26; // ~1 year at 14-day cycles

static int BudgetCycleBlocksFromTargetSpacing(int64_t targetSpacing)
{
    if (targetSpacing <= 0) {
        targetSpacing = 1;
    }
    return static_cast<int>(GOVERNANCE_CYCLE_SECONDS / targetSpacing);
}
} // namespace

/**
 * Build the genesis block.
 *
 * Note: the genesis coinbase output is not spendable in this codebase.
 */
static CBlock Create1776CashGenesisBlock(const char* pszTimestamp, uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    const CScript genesisOutputScript = CScript() << ParseHex("04c10e83b2703ccf322f7dbd62dd5855ac7c10bd055814ce121ba32607d573b8810c02c0582aed05b4deb9c4b77b26d92428c61256cd42774babea0a073b2ed0c9") << OP_CHECKSIG;
    return CreateGenesisBlock(pszTimestamp, genesisOutputScript, nTime, nNonce, nBits, nVersion, genesisReward);
}

static CBlock CreateGenesisBlock(uint32_t nTime, uint32_t nNonce, uint32_t nBits, int32_t nVersion, const CAmount& genesisReward)
{
    return Create1776CashGenesisBlock("1776CASH Genesis 2026-02-02", nTime, nNonce, nBits, nVersion, genesisReward);
}

// this one is for testing only
static Consensus::LLMQParams llmq_test = {
        .type = Consensus::LLMQ_TEST,
        .name = "llmq_test",
        .size = 3,
        .minSize = 2,
        .threshold = 2,

        .dkgInterval = 20, // one every 20 minutes
        .dkgPhaseBlocks = 2,
        .dkgMiningWindowStart = 10, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 15,
        .dkgBadVotesThreshold = 2,

        .signingActiveQuorumCount = 2, // just a few ones to allow easier testing

        .keepOldConnections = 3,
        .recoveryMembers = 3,

        .cacheDkgInterval = 60,
};

static Consensus::LLMQParams llmq50_60 = {
        .type = Consensus::LLMQ_50_60,
        .name = "llmq_50_60",
        .size = 50,
        .minSize = 40,
        .threshold = 30,

        .dkgInterval = 60, // one DKG per hour
        .dkgPhaseBlocks = 6,
        .dkgMiningWindowStart = 30, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 40,
        .dkgBadVotesThreshold = 40,

        .signingActiveQuorumCount = 24, // a full day worth of LLMQs

        .keepOldConnections = 25,
        .recoveryMembers = 25,

        .cacheDkgInterval = 600,
};

static Consensus::LLMQParams llmq400_60 = {
        .type = Consensus::LLMQ_400_60,
        .name = "llmq_400_60",
        .size = 400,
        .minSize = 300,
        .threshold = 240,

        .dkgInterval = 60 * 12, // one DKG every 12 hours
        .dkgPhaseBlocks = 10,
        .dkgMiningWindowStart = 50, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 70,
        .dkgBadVotesThreshold = 300,

        .signingActiveQuorumCount = 4, // two days worth of LLMQs

        .keepOldConnections = 5,
        .recoveryMembers = 100,

        .cacheDkgInterval = 60 * 12 * 10, // dkgInterval * 10
};

// Used for deployment and min-proto-version signaling, so it needs a higher threshold
static Consensus::LLMQParams llmq400_85 = {
        .type = Consensus::LLMQ_400_85,
        .name = "llmq_400_85",
        .size = 400,
        .minSize = 350,
        .threshold = 340,

        .dkgInterval = 60 * 24, // one DKG every 24 hours
        .dkgPhaseBlocks = 10,
        .dkgMiningWindowStart = 50, // dkgPhaseBlocks * 5 = after finalization
        .dkgMiningWindowEnd = 70, // give it a larger mining window to make sure it is mined
        .dkgBadVotesThreshold = 300,

        .signingActiveQuorumCount = 4, // four days worth of LLMQs

        .keepOldConnections = 5,
        .recoveryMembers = 100,

        .cacheDkgInterval = 60 * 24 * 10, // dkgInterval * 10
};

/**
 * Main network
 */
/**
 * What makes a good checkpoint block?
 * + Is surrounded by blocks with reasonable timestamps
 *   (no blocks before with a timestamp after, none after with
 *    timestamp before)
 * + Contains no strange transactions
 */
static MapCheckpoints mapCheckpoints = {
    {0, uint256S("0x000008eda97dd1a0281134beca3ecdd04406006b8fa6222b2d1040b6c4abde6b")},
};

static const CCheckpointData data = {
    &mapCheckpoints,
    1780925434, // * UNIX timestamp of last checkpoint block (genesis)
    0,          // * total number of transactions between genesis and last checkpoint
    1800        // * estimated number of transactions per day after checkpoint
};

static MapCheckpoints mapCheckpointsTestnet = {
    {0, uint256S("0x5c7d058f3d81005a355168d9d0d1e336df090d62140b5ebf5a3b851f80c6a3ef")},
    {5000, uint256S("0x000004cee9054173ff2e8fd8dec5b467816162b3a28ecf0e55e30a5750aec77c")},
    {10000, uint256S("0x1148f648a7afcea6c168e8d5d2f348204d7a9fbf54b043fa0d2def8a5a779f0f")},
    {15000, uint256S("0x12d9ddaf132f1e5e91afc0586ddccc01c0ddc8555559893476bcc418203e8a10")},
    {16000, uint256S("0x7f0134d9900bbd2827dd915d096b1b040e0426adb475286ff5d7e43d753d711f")},
    {17000, uint256S("0xf157a16a86328a7bda09632d00cb5023754448bd19fd93d8907b94a3407bc3cc")},
    {18000, uint256S("0x3c1066035984a8e970148e6b8020f2d3638c61d19d48dccf817e85cfd3f37d93")},
};

static const CCheckpointData dataTestnet = {
    &mapCheckpointsTestnet,
    1770932145,  // timestamp of block 18000
    18000,       // estimated tx count
    500};        // estimated tx per day

static MapCheckpoints mapCheckpointsRegtest = {{0, uint256S("0x7610e035d4e7332401d6b40c7a3bf8bb3445d768823f5fe577921b6cd21ad3b4")}};
static const CCheckpointData dataRegtest = {
    &mapCheckpointsRegtest,
    1768953602,
    1,
    100};

class CMainParams : public CChainParams
{
public:
    CMainParams()
    {
        strNetworkID = "main";

        genesis = Create1776CashGenesisBlock("1776CASH Genesis 2026-06-08", 1780925434, 1042315, 0x1e0ffff0, 1, 0 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x000008eda97dd1a0281134beca3ecdd04406006b8fa6222b2d1040b6c4abde6b"));
        assert(genesis.hashMerkleRoot == uint256S("0x1490cb17a58d8b72d87d6e143bd47a49235669f109e2067afd6863954d9f18e8"));

        consensus.fPowAllowMinDifficultyBlocks = false;
        consensus.fPowNoRetargeting = false;
        consensus.powLimit   = uint256S("0x00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimitV1 = uint256S("0x000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimitV2 = uint256S("0x00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nBudgetFeeConfirmations = 6;      // Number of confirmations for the finalization fee
        consensus.nCoinbaseMaturity = 100;
        consensus.nFutureTimeDriftPoW = 7200;  // 2 hours - generous clock skew tolerance
        consensus.nFutureTimeDriftPoS = 900;   // 15 minutes (7.5 blocks for 2-min blocks) - for network latency
        consensus.nMaxMoneyOut = 177600000 * COIN;
        consensus.nPremineReward = 102632000 * COIN;
        consensus.nBlockSubsidy = 10 * COIN;
        consensus.nMNCollateralAmt = 4000 * COIN;
        // Masternode rewards are only paid after PoS starts (see GetMasternodePayment()).
        consensus.nMNBlockReward = 0 * COIN;
        consensus.nNewMNBlockReward = 6 * COIN;
        consensus.nMNCollateralMinConf = 15;
        consensus.nProposalEstablishmentTime = 60 * 60 * 24;    // must be at least a day old to make it into a budget
        consensus.nStakeMinAge = 60 * 60;
        consensus.nStakeMinDepth = 600;
        consensus.nTargetTimespan = 80 * 60;
        consensus.nTargetTimespanV2 = 60 * 60;
        consensus.nTargetSpacing = 2 * 60;
        consensus.nBudgetCycleBlocks = BudgetCycleBlocksFromTargetSpacing(consensus.nTargetSpacing); // 14 days
        consensus.nBudgetCycleAmount = GOVERNANCE_CYCLE_BUDGET;
        consensus.nTimeSlotLength = 15;
        consensus.nMaxProposalPayments = GOVERNANCE_MAX_CYCLE_PAYMENTS;

        // spork keys
        consensus.strSporkPubKey = "048ac5e30b084e292ca202402449eb475a5c964a2b588cf69a7609ae4900b0c755daf8091cda0dc2d8ac7a3cd2d703f0c1bde6fc81f77a8fae491afe6f214fa869";
        consensus.strSporkPubKeyOld = "";
        consensus.nTime_EnforceNewSporkKey = 0;
        consensus.nTime_RejectOldSporkKey = 0;

        // height-based activations
        consensus.height_last_invalid_UTXO = 894538;
        consensus.height_last_ZC_AccumCheckpoint = 1686240;
        consensus.height_last_ZC_WrappedSerials = 1686229;

        consensus.nPivxBadBlockTime = 0;
        consensus.nPivxBadBlockBits = 0;

        // Zerocoin-related params
        consensus.ZC_Modulus = "25195908475657893494027183240048398571429282126204032027777137836043662020707595556264018525880784"
                "4069182906412495150821892985591491761845028084891200728449926873928072877767359714183472702618963750149718246911"
                "6507761337985909570009733045974880842840179742910064245869181719511874612151517265463228221686998754918242243363"
                "7259085141865462043576798423387184774447920739934236584823824281198163815010674810451660377306056201619676256133"
                "8441436038339044149526344321901146575444541784240209246165157233507787077498171257724679629263863563732899121548"
                "31438167899885040445364023527381951378636564391212010397122822120720357";
        consensus.ZC_MaxPublicSpendsPerTx = 637;    // Assume about 220 bytes each input
        consensus.ZC_MaxSpendsPerTx = 7;            // Assume about 20kb each input
        consensus.ZC_MinMintConfirmations = 20;
        consensus.ZC_MinMintFee = 1 * CENT;
        consensus.ZC_MinStakeDepth = 200;
        consensus.ZC_TimeStart = 1508214600;        // October 17, 2017 4:30:00 AM
        consensus.ZC_HeightStart = 863735;

        // Network upgrades
        consensus.vUpgrades[Consensus::BASE_NETWORK].nActivationHeight =
                Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
                Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_POS].nActivationHeight           = 10081;
        consensus.vUpgrades[Consensus::UPGRADE_POS_V2].nActivationHeight        = 10081;
        consensus.vUpgrades[Consensus::UPGRADE_ZC].nActivationHeight            = Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_ZC_V2].nActivationHeight         = Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_BIP65].nActivationHeight         = Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_ZC_PUBLIC].nActivationHeight     = Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_V3_4].nActivationHeight          = 10081;
        consensus.vUpgrades[Consensus::UPGRADE_V4_0].nActivationHeight          = 10081;
        consensus.vUpgrades[Consensus::UPGRADE_V5_0].nActivationHeight          = 10081;
        consensus.vUpgrades[Consensus::UPGRADE_V5_2].nActivationHeight          = 10081;
        consensus.vUpgrades[Consensus::UPGRADE_V5_3].nActivationHeight          = 10081;
        consensus.vUpgrades[Consensus::UPGRADE_V5_5].nActivationHeight          = 10081;
        consensus.vUpgrades[Consensus::UPGRADE_V5_6].nActivationHeight          = 10081;
        consensus.vUpgrades[Consensus::UPGRADE_V6_0].nActivationHeight          = 10081;
        consensus.vUpgrades[Consensus::UPGRADE_V6_1_GOV].nActivationHeight      = 10081;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0xd1;
        pchMessageStart[1] = 0x4f;
        pchMessageStart[2] = 0x82;
        pchMessageStart[3] = 0x3a;
        nDefaultPort = 17776;

        // Seed nodes (bootstrap). Keep this list small and reliable.
        // DNS seeds (mainnet)
        vSeeds.emplace_back("johny.1776cash.com", true);
        vSeeds.emplace_back("michael.1776cash.com", true);
        // Direct IP seeds (mainnet public listeners)
        vSeeds.emplace_back("46.224.203.241");
        vSeeds.emplace_back("178.156.210.215");
        vSeeds.emplace_back("89.167.55.37");
        vSeeds.emplace_back("5.78.154.213");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 30);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 16);
        base58Prefixes[STAKING_ADDRESS] = std::vector<unsigned char>(1, 28);
        base58Prefixes[EXCHANGE_ADDRESS] = {0x02, 0x17, 0x77};
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 158);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x88, 0xB2, 0x1E};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x88, 0xAD, 0xE4};
        // BIP44 coin type is from https://github.com/satoshilabs/slips/blob/master/slip-0044.md
        base58Prefixes[EXT_COIN_TYPE] = {0x80, 0x00, 0x06, 0xF0}; // 1776

        // Fixed seeds for 1776CASH mainnet (BIP155 format: networkID, length, IP, port)
        // Public listener set: 46.224.203.241, 178.156.210.215, 89.167.55.37, 5.78.154.213
        vFixedSeeds = {
            0x01, 0x04, 0x2E, 0xE0, 0xCB, 0xF1, 0x45, 0x70,  // 46.224.203.241:17776
            0x01, 0x04, 0xB2, 0x9C, 0xD2, 0xD7, 0x45, 0x70,  // 178.156.210.215:17776
            0x01, 0x04, 0x59, 0xA7, 0x37, 0x25, 0x45, 0x70,  // 89.167.55.37:17776
            0x01, 0x04, 0x05, 0x4E, 0x9A, 0xD5, 0x45, 0x70,  // 5.78.154.213:17776
        };

        // Reject non-standard transactions by default
        fRequireStandard = true;

        // Sapling
        bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "c8";
        bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "c1776v2views";
        bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "c1776v2ivks";
        bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "c1776-secret-spending-key-v2";
        bech32HRPs[SAPLING_EXTENDED_FVK]         = "c1776v2xviews";

        bech32HRPs[BLS_SECRET_KEY]               = "c1776v2-bls-sk";
        bech32HRPs[BLS_PUBLIC_KEY]               = "c1776v2-bls-pk";

        // long living quorum params
        consensus.llmqs[Consensus::LLMQ_50_60] = llmq50_60;
        consensus.llmqs[Consensus::LLMQ_400_60] = llmq400_60;
        consensus.llmqs[Consensus::LLMQ_400_85] = llmq400_85;
        consensus.llmqs[Consensus::LLMQ_TEST] = llmq_test;

        nLLMQConnectionRetryTimeout = 60;

        // Mainnet V4 is activated at height 10081, use test-sized quorum type for early network rollout.
        consensus.llmqTypeChainLocks = Consensus::LLMQ_TEST;

        // Tier two
        nFulfilledRequestExpireTime = 60 * 60; // fulfilled requests expire in 1 hour
    }

    const CCheckpointData& Checkpoints() const
    {
        return data;
    }

};

/**
 * Testnet (v5)
 */
class CTestNetParams : public CChainParams
{
public:
    CTestNetParams()
    {
        strNetworkID = "test";

        genesis = CreateGenesisBlock(1768953601, 55811, 0x1e0ffff0, 1, 0 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x0000015778bd1bbe045e86b96899fd1f016d48f352c9197632e30d99ef809c3c"));
        assert(genesis.hashMerkleRoot == uint256S("0xdf8d63cd52589c48295907a5babc3d4eb9d4a95c1a48e092b8ca408a115c51ff"));

        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = false;
        consensus.powLimit   = uint256S("0x00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimitV1 = uint256S("0x000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimitV2 = uint256S("0x00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nBudgetFeeConfirmations = 3;      // (only 8-blocks window for finalization on testnet)
        consensus.nCoinbaseMaturity = 15;
        consensus.nFutureTimeDriftPoW = 600;  // Increased for better tolerance across regions
        consensus.nFutureTimeDriftPoS = 900;  // Increased for better tolerance across regions
        consensus.nMaxMoneyOut = 177600000 * COIN;
        consensus.nMNCollateralAmt = 4000 * COIN;
        // Masternode rewards are only paid after PoS starts (see GetMasternodePayment()).
        consensus.nMNBlockReward = 0 * COIN;
        consensus.nNewMNBlockReward = 6 * COIN;
        consensus.nMNCollateralMinConf = 15;
        consensus.nProposalEstablishmentTime = 60 * 5;  // at least 5 min old to make it into a budget
        // Testnet is tuned for fast iteration: 30s blocks, 4x faster staking requirements.
        consensus.nStakeMinAge = 15 * 60;
        consensus.nStakeMinDepth = 25;
        consensus.nTargetTimespan = 80 * 60;
        consensus.nTargetTimespanV2 = 60 * 60;
        consensus.nTargetSpacing = 30;
        consensus.nBudgetCycleBlocks = BudgetCycleBlocksFromTargetSpacing(consensus.nTargetSpacing); // 14 days
        consensus.nBudgetCycleAmount = GOVERNANCE_CYCLE_BUDGET;
        consensus.nTimeSlotLength = 15;
        consensus.nMaxProposalPayments = GOVERNANCE_MAX_CYCLE_PAYMENTS;

        // spork keys
        consensus.strSporkPubKey = "04677c34726c491117265f4b1c83cef085684f36c8df5a97a3a42fc499316d0c4e63959c9eca0dba239d9aaaf72011afffeb3ef9f51b9017811dec686e412eb504";
        consensus.strSporkPubKeyOld = "04E88BB455E2A04E65FCC41D88CD367E9CCE1F5A409BE94D8C2B4B35D223DED9C8E2F4E061349BA3A38839282508066B6DC4DB72DD432AC4067991E6BF20176127";
        consensus.nTime_EnforceNewSporkKey = 1608512400;    //!> December 21, 2020 01:00:00 AM GMT
        consensus.nTime_RejectOldSporkKey = 1614560400;     //!> March 1, 2021 01:00:00 AM GMT

        // height based activations
        consensus.height_last_invalid_UTXO = -1;
        consensus.height_last_ZC_AccumCheckpoint = -1;
        consensus.height_last_ZC_WrappedSerials = -1;
        consensus.ZC_HeightStart = 0;

        // Zerocoin-related params
        consensus.ZC_Modulus = "25195908475657893494027183240048398571429282126204032027777137836043662020707595556264018525880784"
                "4069182906412495150821892985591491761845028084891200728449926873928072877767359714183472702618963750149718246911"
                "6507761337985909570009733045974880842840179742910064245869181719511874612151517265463228221686998754918242243363"
                "7259085141865462043576798423387184774447920739934236584823824281198163815010674810451660377306056201619676256133"
                "8441436038339044149526344321901146575444541784240209246165157233507787077498171257724679629263863563732899121548"
                "31438167899885040445364023527381951378636564391212010397122822120720357";
        consensus.ZC_MaxPublicSpendsPerTx = 637;    // Assume about 220 bytes each input
        consensus.ZC_MaxSpendsPerTx = 7;            // Assume about 20kb each input
        consensus.ZC_MinMintConfirmations = 20;
        consensus.ZC_MinMintFee = 1 * CENT;
        consensus.ZC_MinStakeDepth = 200;
        consensus.ZC_TimeStart = 1508214600;        // October 17, 2017 4:30:00 AM

        // Network upgrades
        consensus.vUpgrades[Consensus::BASE_NETWORK].nActivationHeight =
                Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
                Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;

        consensus.vUpgrades[Consensus::UPGRADE_POS].nActivationHeight           = 5041;
        consensus.vUpgrades[Consensus::UPGRADE_POS_V2].nActivationHeight        = 32000;
        consensus.vUpgrades[Consensus::UPGRADE_ZC].nActivationHeight            = 32200;
        consensus.vUpgrades[Consensus::UPGRADE_ZC_V2].nActivationHeight         = 32400;
        consensus.vUpgrades[Consensus::UPGRADE_BIP65].nActivationHeight         = 32600;
        consensus.vUpgrades[Consensus::UPGRADE_ZC_PUBLIC].nActivationHeight     = 32800;
        consensus.vUpgrades[Consensus::UPGRADE_V3_4].nActivationHeight          = 33000;
        consensus.vUpgrades[Consensus::UPGRADE_V4_0].nActivationHeight          = 33200;
        consensus.vUpgrades[Consensus::UPGRADE_V5_0].nActivationHeight          = 33400;
        consensus.vUpgrades[Consensus::UPGRADE_V5_2].nActivationHeight          = 33600;
        consensus.vUpgrades[Consensus::UPGRADE_V5_3].nActivationHeight          = 33800;
        consensus.vUpgrades[Consensus::UPGRADE_V5_5].nActivationHeight          = 34000;
        consensus.vUpgrades[Consensus::UPGRADE_V5_6].nActivationHeight          = 34200;
        consensus.vUpgrades[Consensus::UPGRADE_V6_0].nActivationHeight          = 34400;
        consensus.vUpgrades[Consensus::UPGRADE_V6_1_GOV].nActivationHeight      = 41000;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0xb2;
        pchMessageStart[1] = 0x17;
        pchMessageStart[2] = 0x76;
        pchMessageStart[3] = 0xca;
        nDefaultPort = 27776;

        // Seed nodes (bootstrap). Keep this list small and reliable.
        // DNS seeds (testnet)
        vSeeds.emplace_back("jimmy.1776cash.com", true);
        // Direct IP seeds (testnet)
        vSeeds.emplace_back("65.108.53.80");
        vSeeds.emplace_back("195.201.34.89");
        vSeeds.emplace_back("46.62.255.185");

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 38);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 25);
        base58Prefixes[STAKING_ADDRESS] = std::vector<unsigned char>(1, 57);
        base58Prefixes[EXCHANGE_ADDRESS] = {0x02, 0x17, 0x76};
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 184);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x17, 0x76, 0xCA};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x17, 0x76, 0xCB};
        // Testnet pivx BIP44 coin type is '1' (All coin's testnet default)
        base58Prefixes[EXT_COIN_TYPE] = {0x80, 0x00, 0x00, 0x01};

        // Fixed seeds for 1776CASH testnet (BIP155 format: networkID, length, IP, port)
        // 65.108.53.80:27776
        vFixedSeeds = {
            0x01, 0x04, 0x41, 0x6C, 0x35, 0x50, 0x6C, 0x80,  // 65.108.53.80:27776
        };

        fRequireStandard = false;

        // Sapling - use "tc7" prefix for testnet to distinguish from mainnet
        bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "tc7";
        bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "tc1776views";
        bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "tc1776ivks";
        bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "c1776-secret-spending-key-test";
        bech32HRPs[SAPLING_EXTENDED_FVK]         = "tc1776xviews";

        bech32HRPs[BLS_SECRET_KEY]               = "c1776-bls-sk-test";
        bech32HRPs[BLS_PUBLIC_KEY]               = "c1776-bls-pk-test";

        // long living quorum params
        consensus.llmqs[Consensus::LLMQ_50_60] = llmq50_60;
        consensus.llmqs[Consensus::LLMQ_400_60] = llmq400_60;
        consensus.llmqs[Consensus::LLMQ_400_85] = llmq400_85;
        consensus.llmqs[Consensus::LLMQ_TEST] = llmq_test;

        nLLMQConnectionRetryTimeout = 60;

        consensus.llmqTypeChainLocks = Consensus::LLMQ_TEST;

        // Tier two
        nFulfilledRequestExpireTime = 60 * 60; // fulfilled requests expire in 1 hour
    }

    const CCheckpointData& Checkpoints() const
    {
        return dataTestnet;
    }
};

/**
 * Regression test
 */
class CRegTestParams : public CChainParams
{
public:
    CRegTestParams()
    {
        strNetworkID = "regtest";

        genesis = CreateGenesisBlock(1768953602, 0, 0x207fffff, 1, 0 * COIN);
        consensus.hashGenesisBlock = genesis.GetHash();
        assert(consensus.hashGenesisBlock == uint256S("0x7610e035d4e7332401d6b40c7a3bf8bb3445d768823f5fe577921b6cd21ad3b4"));
        assert(genesis.hashMerkleRoot == uint256S("0xdf8d63cd52589c48295907a5babc3d4eb9d4a95c1a48e092b8ca408a115c51ff"));

        consensus.fPowAllowMinDifficultyBlocks = true;
        consensus.fPowNoRetargeting = true;
        consensus.powLimit   = uint256S("0x7fffffffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimitV1 = uint256S("0x000000ffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.posLimitV2 = uint256S("0x00000fffffffffffffffffffffffffffffffffffffffffffffffffffffffffff");
        consensus.nBudgetCycleBlocks = 144;         // approx 10 cycles per day
        consensus.nBudgetFeeConfirmations = 3;      // (only 8-blocks window for finalization on regtest)
        consensus.nCoinbaseMaturity = 100;
        consensus.nFutureTimeDriftPoW = 7200;
        consensus.nFutureTimeDriftPoS = 180;
        consensus.nMaxMoneyOut = 177600000 * COIN;
        consensus.nMNCollateralAmt = 100 * COIN;
        consensus.nMNBlockReward = 0 * COIN;
        consensus.nNewMNBlockReward = 6 * COIN;
        consensus.nMNCollateralMinConf = 1;
        consensus.nProposalEstablishmentTime = 60 * 5;  // at least 5 min old to make it into a budget
        consensus.nStakeMinAge = 0;
        consensus.nStakeMinDepth = 20;
        consensus.nTargetTimespan = 80 * 60;
        consensus.nTargetTimespanV2 = 60 * 60;
        consensus.nTargetSpacing = 2 * 60;
        consensus.nBudgetCycleAmount = GOVERNANCE_CYCLE_BUDGET;
        consensus.nTimeSlotLength = 15;
        consensus.nMaxProposalPayments = 20;

        /* Spork Key for RegTest:
        WIF private key: 932HEevBSujW2ud7RfB1YF91AFygbBRQj3de3LyaCRqNzKKgWXi
        private key hex: bd4960dcbd9e7f2223f24e7164ecb6f1fe96fc3a416f5d3a830ba5720c84b8ca
        Address: yCvUVd72w7xpimf981m114FSFbmAmne7j9
        */
        consensus.strSporkPubKey = "043969b1b0e6f327de37f297a015d37e2235eaaeeb3933deecd8162c075cee0207b13537618bde640879606001a8136091c62ec272dd0133424a178704e6e75bb7";
        consensus.strSporkPubKeyOld = "";
        consensus.nTime_EnforceNewSporkKey = 0;
        consensus.nTime_RejectOldSporkKey = 0;

        // height based activations
        consensus.height_last_invalid_UTXO = -1;
        consensus.height_last_ZC_AccumCheckpoint = 310;     // no checkpoints on regtest
        consensus.height_last_ZC_WrappedSerials = -1;

        // Zerocoin-related params
        consensus.ZC_Modulus = "25195908475657893494027183240048398571429282126204032027777137836043662020707595556264018525880784"
                "4069182906412495150821892985591491761845028084891200728449926873928072877767359714183472702618963750149718246911"
                "6507761337985909570009733045974880842840179742910064245869181719511874612151517265463228221686998754918242243363"
                "7259085141865462043576798423387184774447920739934236584823824281198163815010674810451660377306056201619676256133"
                "8441436038339044149526344321901146575444541784240209246165157233507787077498171257724679629263863563732899121548"
                "31438167899885040445364023527381951378636564391212010397122822120720357";
        consensus.ZC_MaxPublicSpendsPerTx = 637;    // Assume about 220 bytes each input
        consensus.ZC_MaxSpendsPerTx = 7;            // Assume about 20kb each input
        consensus.ZC_MinMintConfirmations = 10;
        consensus.ZC_MinMintFee = 1 * CENT;
        consensus.ZC_MinStakeDepth = 10;
        consensus.ZC_TimeStart = 0;                 // not implemented on regtest
        consensus.ZC_HeightStart = 0;

        // Network upgrades
        consensus.vUpgrades[Consensus::BASE_NETWORK].nActivationHeight =
                Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_TESTDUMMY].nActivationHeight =
                Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_POS].nActivationHeight           = 251;
        consensus.vUpgrades[Consensus::UPGRADE_POS_V2].nActivationHeight        = 251;
        consensus.vUpgrades[Consensus::UPGRADE_ZC].nActivationHeight            = 300;
        consensus.vUpgrades[Consensus::UPGRADE_ZC_V2].nActivationHeight         = 300;
        consensus.vUpgrades[Consensus::UPGRADE_BIP65].nActivationHeight         =
                Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_ZC_PUBLIC].nActivationHeight     = 400;
        consensus.vUpgrades[Consensus::UPGRADE_V3_4].nActivationHeight          = 251;
        consensus.vUpgrades[Consensus::UPGRADE_V4_0].nActivationHeight          =
                Consensus::NetworkUpgrade::ALWAYS_ACTIVE;
        consensus.vUpgrades[Consensus::UPGRADE_V5_0].nActivationHeight          = 300;
        consensus.vUpgrades[Consensus::UPGRADE_V5_2].nActivationHeight          = 300;
        consensus.vUpgrades[Consensus::UPGRADE_V5_3].nActivationHeight          = 251;
        consensus.vUpgrades[Consensus::UPGRADE_V5_5].nActivationHeight          = 576;
        consensus.vUpgrades[Consensus::UPGRADE_V5_6].nActivationHeight          = 1000;
        consensus.vUpgrades[Consensus::UPGRADE_V6_0].nActivationHeight =
                Consensus::NetworkUpgrade::NO_ACTIVATION_HEIGHT;
        consensus.vUpgrades[Consensus::UPGRADE_V6_1_GOV].nActivationHeight =
                Consensus::NetworkUpgrade::ALWAYS_ACTIVE;

        /**
         * The message start string is designed to be unlikely to occur in normal data.
         * The characters are rarely used upper ASCII, not valid as UTF-8, and produce
         * a large 4-byte int at any alignment.
         */
        pchMessageStart[0] = 0xb3;
        pchMessageStart[1] = 0x17;
        pchMessageStart[2] = 0x76;
        pchMessageStart[3] = 0xca;
        nDefaultPort = 51476;

        base58Prefixes[PUBKEY_ADDRESS] = std::vector<unsigned char>(1, 38);
        base58Prefixes[SCRIPT_ADDRESS] = std::vector<unsigned char>(1, 25);
        base58Prefixes[STAKING_ADDRESS] = std::vector<unsigned char>(1, 57);
        base58Prefixes[EXCHANGE_ADDRESS] = {0x02, 0x17, 0x76};
        base58Prefixes[SECRET_KEY] = std::vector<unsigned char>(1, 184);
        base58Prefixes[EXT_PUBLIC_KEY] = {0x04, 0x17, 0x76, 0xCA};
        base58Prefixes[EXT_SECRET_KEY] = {0x04, 0x17, 0x76, 0xCB};
        // Testnet pivx BIP44 coin type is '1' (All coin's testnet default)
        base58Prefixes[EXT_COIN_TYPE] = {0x80, 0x00, 0x00, 0x01};

        // Reject non-standard transactions by default
        fRequireStandard = true;

        // Sapling - use "rc7" prefix for regtest to distinguish from mainnet/testnet
        bech32HRPs[SAPLING_PAYMENT_ADDRESS]      = "rc7";
        bech32HRPs[SAPLING_FULL_VIEWING_KEY]     = "rc1776views";
        bech32HRPs[SAPLING_INCOMING_VIEWING_KEY] = "rc1776ivks";
        bech32HRPs[SAPLING_EXTENDED_SPEND_KEY]   = "c1776-secret-spending-key-regtest";
        bech32HRPs[SAPLING_EXTENDED_FVK]         = "rc1776xviews";

        bech32HRPs[BLS_SECRET_KEY]               = "c1776-bls-sk-regtest";
        bech32HRPs[BLS_PUBLIC_KEY]               = "c1776-bls-pk-regtest";

        // long living quorum params
        consensus.llmqs[Consensus::LLMQ_TEST] = llmq_test;
        nLLMQConnectionRetryTimeout = 10;

        consensus.llmqTypeChainLocks = Consensus::LLMQ_TEST;

        // Tier two
        nFulfilledRequestExpireTime = 60 * 60; // fulfilled requests expire in 1 hour
    }

    const CCheckpointData& Checkpoints() const
    {
        return dataRegtest;
    }
};

static std::unique_ptr<CChainParams> globalChainParams;

const CChainParams &Params()
{
    assert(globalChainParams);
    return *globalChainParams;
}

std::unique_ptr<CChainParams> CreateChainParams(const std::string& chain)
{
    if (chain == CBaseChainParams::MAIN)
        return std::unique_ptr<CChainParams>(new CMainParams());
    else if (chain == CBaseChainParams::TESTNET)
        return std::unique_ptr<CChainParams>(new CTestNetParams());
    else if (chain == CBaseChainParams::REGTEST)
        return std::unique_ptr<CChainParams>(new CRegTestParams());
    throw std::runtime_error(strprintf("%s: Unknown chain %s.", __func__, chain));
}

void SelectParams(const std::string& network)
{
    SelectBaseParams(network);
    globalChainParams = CreateChainParams(network);
}

void UpdateNetworkUpgradeParameters(Consensus::UpgradeIndex idx, int nActivationHeight)
{
    globalChainParams->UpdateNetworkUpgradeParameters(idx, nActivationHeight);
}
