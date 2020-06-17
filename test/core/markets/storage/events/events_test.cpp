/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#include <gtest/gtest.h>
#include "markets/storage/events/impl/events_impl.hpp"
#include "storage/ipfs/impl/in_memory_datastore.hpp"
#include "testutil/literals.hpp"
#include "testutil/outcome.hpp"
#include "vm/actor/builtin/miner/miner_actor.hpp"

namespace fc::markets::storage::events {
  using adt::Channel;
  using api::Api;
  using api::Chan;
  using fc::storage::ipfs::InMemoryDatastore;
  using fc::storage::ipfs::IpfsDatastore;
  using primitives::block::BlockHeader;
  using primitives::block::MsgMeta;
  using primitives::tipset::HeadChange;
  using primitives::tipset::HeadChangeType;
  using primitives::tipset::Tipset;
  using vm::actor::MethodParams;
  using vm::actor::builtin::miner::PreCommitSector;
  using vm::actor::builtin::miner::ProveCommitSector;
  using vm::actor::builtin::miner::SectorPreCommitInfo;
  using vm::message::SignedMessage;
  using vm::message::UnsignedMessage;

  class EventsTest : public ::testing::Test {
   public:
    void SetUp() override {
      api = std::make_shared<Api>();
      events = std::make_shared<EventsImpl>(api, ipld);
    }

    Address provider = Address::makeFromId(1);
    DealId deal_id{1};
    SectorNumber sector_number{13};
    std::shared_ptr<Api> api;
    std::shared_ptr<IpfsDatastore> ipld = std::make_shared<InMemoryDatastore>();
    std::shared_ptr<EventsImpl> events;
  };

  /**
   * @given subscription to events by address and deal id
   * @when PreCommit and then ProveCommit called
   * @then event is triggered
   */
  TEST_F(EventsTest, CommitSector) {
    api->ChainNotify = {[=]()
                            -> outcome::result<Chan<std::vector<HeadChange>>> {
      auto channel{std::make_shared<Channel<std::vector<HeadChange>>>()};

      // PreCommitSector message call
      SectorPreCommitInfo pre_commit_info;
      pre_commit_info.sealed_cid = "010001020001"_cid;
      pre_commit_info.deal_ids.emplace_back(deal_id);
      pre_commit_info.sector = sector_number;
      OUTCOME_TRY(pre_commit_params, codec::cbor::encode(pre_commit_info));
      UnsignedMessage pre_commit_message;
      pre_commit_message.to = provider;
      pre_commit_message.method = PreCommitSector::Number;
      pre_commit_message.params = MethodParams{pre_commit_params};
      OUTCOME_TRY(pre_commit_message_cid, ipld->setCbor(pre_commit_message));

      // ProveCommitSector message call
      ProveCommitSector::Params prove_commit_param;
      prove_commit_param.sector = sector_number;
      OUTCOME_TRY(encoded_prove_commit_params,
                  codec::cbor::encode(prove_commit_param));
      UnsignedMessage prove_commit_message;
      prove_commit_message.to = provider;
      prove_commit_message.method = ProveCommitSector::Number;
      prove_commit_message.params = MethodParams{encoded_prove_commit_params};
      OUTCOME_TRY(prove_commit_message_cid,
                  ipld->setCbor(prove_commit_message));

      MsgMeta meta;
      ipld->load(meta);
      OUTCOME_TRY(meta.bls_messages.append(pre_commit_message_cid));
      OUTCOME_TRY(meta.bls_messages.append(prove_commit_message_cid));
      OUTCOME_TRY(messages, ipld->setCbor(meta));
      BlockHeader block_header{.messages = messages};

      Tipset tipset{.blks = {block_header}};
      HeadChange change{.type = HeadChangeType::APPLY, .value = tipset};
      channel->write({change});

      return Chan{std::move(channel)};
    }};

    auto res = events->onDealSectorCommitted(provider, deal_id);

    EXPECT_OUTCOME_TRUE_1(events->init());

    auto future = res->get_future();
    EXPECT_EQ(std::future_status::ready,
              future.wait_for(std::chrono::seconds(0)));
    EXPECT_OUTCOME_TRUE_1(future.get());
  }

  /**
   * @given call onDealSectorCommitted
   * @when no message committed
   * @then future in wait status
   */
  TEST_F(EventsTest, WaitCommitSector) {
    api->ChainNotify = {
        [=]() -> outcome::result<Chan<std::vector<HeadChange>>> {
          auto channel{std::make_shared<Channel<std::vector<HeadChange>>>()};
          return Chan{std::move(channel)};
        }};

    EXPECT_OUTCOME_TRUE_1(events->init());
    auto res = events->onDealSectorCommitted(provider, deal_id);
    auto future = res->get_future();
    EXPECT_EQ(std::future_status::timeout,
              future.wait_for(std::chrono::seconds(0)));
  }

}  // namespace fc::markets::storage::events
