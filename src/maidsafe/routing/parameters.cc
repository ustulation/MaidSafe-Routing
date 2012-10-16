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

#include "maidsafe/routing/parameters.h"

namespace bptime = boost::posix_time;

namespace maidsafe {

namespace routing {

uint16_t Parameters::thread_count(8);
uint16_t Parameters::num_chunks_to_cache(100);
uint16_t Parameters::closest_nodes_size(8);
uint16_t Parameters::node_group_size(4);
uint16_t Parameters::max_routing_table_size(64);
uint16_t Parameters::routing_table_size_threshold(max_routing_table_size / 2);
uint16_t Parameters::max_client_routing_table_size(8);
uint16_t Parameters::max_non_routing_table_size(64);
uint16_t Parameters::bucket_target_size(1);
uint32_t Parameters::max_data_size(67107840);
Timeout Parameters::find_node_interval(bptime::seconds(10));
Timeout Parameters::recovery_time_lag(bptime::seconds(5));
Timeout Parameters::re_bootstrap_time_lag(bptime::seconds(10));
Timeout Parameters::find_close_node_interval(bptime::seconds(3));
uint16_t Parameters::maximum_find_close_node_failures(10);
uint16_t Parameters::max_route_history(5);
uint16_t Parameters::hops_to_live(20);
Timeout Parameters::connect_rpc_prune_timeout(bptime::seconds(10));

}  // namespace routing

}  // namespace maidsafe
