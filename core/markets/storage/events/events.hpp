/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CPP_FILECOIN_CORE_MARKETS_STORAGE_EVENTS_HPP
#define CPP_FILECOIN_CORE_MARKETS_STORAGE_EVENTS_HPP

#include <future>
#include "storage/mpool/mpool.hpp"

namespace fc::markets::storage::events {
  using fc::storage::mpool::MpoolUpdate;
  using primitives::DealId;
  using primitives::address::Address;
  using primitives::SectorNumber;

  /**
   * Watches for a specified method on an actor is called
   */
  class Events {
   public:
    using PromiseResult = std::promise<outcome::result<void>>;

    struct EventWatch {
      Address provider;
      DealId deal_id;
      boost::optional<SectorNumber> sector_number;
      std::shared_ptr<PromiseResult> result;
    };

    virtual ~Events() = default;

    virtual outcome::result<void> run() = 0;

    virtual void stop() = 0;

    /**
     * Returnes promise with result when miner actor DealSectorCommitted is
     * called
     * @param provider - provider address
     * @param deal_id - id of committed deal
     * @return promise of method invocations
     */
    virtual std::shared_ptr<PromiseResult> onDealSectorCommitted(
        const Address &provider, const DealId &deal_id) = 0;
  };
}  // namespace fc::markets::storage::events

#endif  // CPP_FILECOIN_CORE_MARKETS_STORAGE_EVENTS_HPP
