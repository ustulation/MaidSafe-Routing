/*  Copyright 2014 MaidSafe.net limited

    This MaidSafe Software is licensed to you under (1) the MaidSafe.net Commercial License,
    version 1.0 or later, or (2) The General Public License (GPL), version 3, depending on which
    licence you accepted on initial access to the Software (the "Licences").

    By contributing code to the MaidSafe Software, or to this project generally, you agree to be
    bound by the terms of the MaidSafe Contributor Agreement, version 1.0, found in the root
    directory of this project at LICENSE, COPYING and CONTRIBUTOR respectively and also
    available at: http://www.maidsafe.net/licenses

    Unless required by applicable law or agreed to in writing, the MaidSafe Software distributed
    under the GPL Licence is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS
    OF ANY KIND, either express or implied.

    See the Licences for the specific language governing permissions and limitations relating to
    use of the MaidSafe Software.                                                                 */

#ifndef MAIDSAFE_ROUTING_MESSAGES_CONNECT_H_
#define MAIDSAFE_ROUTING_MESSAGES_CONNECT_H_

#include "maidsafe/common/config.h"
#include "maidsafe/common/utils.h"
#include "maidsafe/common/serialisation/compile_time_mapper.h"
#include "maidsafe/rudp/contact.h"

#include "maidsafe/routing/message_header.h"
#include "maidsafe/routing/types.h"
#include "maidsafe/routing/utils.h"
#include "maidsafe/routing/messages/messages_fwd.h"

namespace maidsafe {

namespace routing {

struct Connect {
  static const SerialisableTypeTag kSerialisableTypeTag =
      static_cast<SerialisableTypeTag>(MessageTypeTag::kConnect);

  Connect() = default;

  Connect(const Connect&) = delete;

  Connect(Connect&& other) MAIDSAFE_NOEXCEPT
      : header(std::move(other.header)),
        requester_endpoints(std::move(other.requester_endpoints)),
        receiver_id(std::move(other.receiver_id)) {}

  Connect(DestinationAddress destination, SourceAddress source,
          rudp::EndpointPair requester_endpoints, Address receiver_id_in)
      : header(std::move(destination), std::move(source), MessageId(RandomUint32())),
        requester_endpoints(std::move(requester_endpoints)),
        receiver_id(std::move(receiver_id_in)) {}

  explicit Connect(MessageHeader header_in)
      : header(std::move(header_in)), requester_endpoints(), receiver_id() {}

  ~Connect() = default;

  Connect& operator=(const Connect&) = delete;

  Connect& operator=(Connect&& other) MAIDSAFE_NOEXCEPT {
    header = std::move(other.header);
    requester_endpoints = std::move(other.requester_endpoints);
    receiver_id = std::move(other.receiver_id);
    return *this;
  };

  template <typename Archive>
  void save(Archive& archive) const {
    auto payload = Serialise(requester_endpoints, receiver_id);
    header.checksums.front() = MurmurHash2(payload);
    archive(header, kSerialisableTypeTag, payload);
  }

  template <typename Archive>
  void load(Archive& archive) {
    if (!header.source->IsValid()) {
      LOG(kError) << "Invalid header.";
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
    }
    SerialisedData payload;
    archive(payload);
    if (MurmurHash2(payload) != header.checksums.at(header.checksum_index)) {
      LOG(kError) << "Checksum failure.";
      BOOST_THROW_EXCEPTION(MakeError(CommonErrors::parsing_error));
    }
    Parse(payload, requester_endpoints, receiver_id);
  }

  mutable MessageHeader header;
  rudp::EndpointPair requester_endpoints;
  Address receiver_id;
};

}  // namespace routing

}  // namespace maidsafe

#endif  // MAIDSAFE_ROUTING_MESSAGES_CONNECT_H_