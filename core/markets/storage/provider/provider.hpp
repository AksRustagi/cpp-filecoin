/**
 * Copyright Soramitsu Co., Ltd. All Rights Reserved.
 * SPDX-License-Identifier: Apache-2.0
 */

#ifndef CPP_FILECOIN_MARKETS_STORAGE_PROVIDER_PROVIDER_HPP
#define CPP_FILECOIN_MARKETS_STORAGE_PROVIDER_PROVIDER_HPP

#include "markets/storage/provider.hpp"
#include "markets/storage/stored_ask.hpp"

namespace fc::markets::storage::provider {

  class StorageProviderImpl : public StorageProvider {
   public:
    // TODO constructor

    virtual auto addAsk(const TokenAmount &price, ChainEpoch duration)
        -> outcome::result<void>;

    virtual auto listAsks(const Address &address)
        -> outcome::result<std::vector<std::shared_ptr<SignedStorageAsk>>>;

    virtual auto listDeals() -> outcome::result<std::vector<StorageDeal>>;

    virtual auto listIncompleteDeals()
        -> outcome::result<std::vector<MinerDeal>>;

    virtual auto addStorageCollateral(const TokenAmount &amount)
        -> outcome::result<void>;

    virtual auto getStorageCollateral() -> outcome::result<TokenAmount>;

    virtual auto importDataForDeal(const CID &prop_cid,
                                   const libp2p::connection::Stream &data)
        -> outcome::result<void>;

   private:
    std::shared_ptr<StoredAsk> stored_ask_;
  };

}  // namespace fc::markets::storage::provider

#endif  // CPP_FILECOIN_MARKETS_STORAGE_PROVIDER_PROVIDER_HPP
