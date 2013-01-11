/*******************************************************************************
 *  Copyright 2012 maidsafe.net limited                                        *
 *                                                                             *
 *  The following source code is property of maidsafe.net limited and is not   *
 *  meant for external use.  The use of this code is governed by the licence   *
 *  file licence.txt found in the root of this directory and also on           *
 *  www.maidsafe.net.                                                          *
 *                                                                             *
 *  You are not free to copy, amend or otherwise use this source code without  *
 *  the explicit written permission of the board of directors of maidsafe.net. *
 ******************************************************************************/

#include "maidsafe/routing/tests/mock_routing_table.h"

namespace maidsafe {

namespace routing {

namespace test {

MockRoutingTable::MockRoutingTable(bool client_mode, const NodeId& node_id,
                                   const asymm::Keys& keys, NetworkStatistics& network_statistics)
    : RoutingTable(client_mode, node_id, keys, network_statistics) {}

MockRoutingTable::~MockRoutingTable() {}

}  // namespace test

}  // namespace routing

}  // namespace maidsafe


