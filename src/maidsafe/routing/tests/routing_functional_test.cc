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

#include <vector>

#include "maidsafe/rudp/nat_type.h"

#include "maidsafe/routing/tests/routing_network.h"
#include "maidsafe/routing/tests/test_utils.h"

namespace maidsafe {

namespace routing {

namespace test {

class RoutingNetworkTest : public testing::Test {
 public:
  RoutingNetworkTest(void) : env_(NodesEnvironment::g_environment()) {}

  void SetUp() {
    EXPECT_TRUE(env_->RestoreComposition());
    EXPECT_TRUE(env_->WaitForHealthToStabilise());
  }

  void TearDown() {
    EXPECT_LE(kServerSize, env_->ClientIndex());
    EXPECT_LE(kNetworkSize, env_->nodes_.size());
    EXPECT_TRUE(env_->RestoreComposition());
  }

 protected:
  std::shared_ptr<GenericNetwork> env_;
};

TEST_F(RoutingNetworkTest, FUNC_SanityCheck) {
  {
    EXPECT_TRUE(env_->Send(3));
    // This sleep is required for un-responded requests
    Sleep(boost::posix_time::seconds(static_cast<long>(env_->nodes_.size() + 1)));  // NOLINT (Fraser)
    env_->ClearMessages();
  }
  {
    //  GroupSend
    uint16_t random_node(env_->RandomVaultIndex());
    NodeId target_id(env_->nodes_[random_node]->node_id());
    std::vector<NodeId> group_Ids(env_->GetGroupForId(target_id));
    EXPECT_TRUE(env_->GroupSend(target_id, 1));
    for (auto& group_id : group_Ids)
      EXPECT_EQ(1, env_->nodes_.at(env_->NodeIndex(group_id))->MessagesSize());
    env_->ClearMessages();

    // GroupSend SelfId
    EXPECT_TRUE(env_->GroupSend(target_id, 1, random_node));
    for (auto& group_id : group_Ids)
      EXPECT_EQ(1, env_->nodes_.at(env_->NodeIndex(group_id))->MessagesSize());
    env_->ClearMessages();

    // Client groupsend
    EXPECT_TRUE(env_->GroupSend(target_id, 1, kNetworkSize - 1));
    for (auto& group_id : group_Ids)
      EXPECT_EQ(1, env_->nodes_.at(env_->NodeIndex(group_id))->MessagesSize());
    env_->ClearMessages();

    // GroupSend RandomId
    target_id = NodeId(NodeId::kRandomId);
    group_Ids = env_->GetGroupForId(target_id);
    EXPECT_TRUE(env_->GroupSend(target_id, 1));
    for (auto& group_id : group_Ids)
      EXPECT_EQ(1, env_->nodes_.at(env_->NodeIndex(group_id))->MessagesSize());
    env_->ClearMessages();
  }
  {
    // Join client with same Id
    env_->AddNode(true, env_->nodes_[env_->RandomClientIndex()]->node_id());

    // Send to client with same Id
    EXPECT_TRUE(env_->Send(env_->nodes_[kNetworkSize],
                           env_->nodes_[kNetworkSize]->node_id(),
                           true));
    env_->ClearMessages();
  }
  {
    // Anonymous join
    env_->AddNode(true, NodeId(), true);

    // Anonymous group send
    NodeId target_id(NodeId::kRandomId);
    std::vector<NodeId> group_Ids(env_->GetGroupForId(target_id));
    EXPECT_TRUE(env_->GroupSend(target_id, 1, static_cast<uint16_t>(env_->nodes_.size() - 1)));
    for (auto& group_id : group_Ids)
      EXPECT_EQ(1, env_->nodes_.at(env_->NodeIndex(group_id))->MessagesSize());
    env_->ClearMessages();
  }
}

TEST_F(RoutingNetworkTest, FUNC_Send) {
  EXPECT_TRUE(env_->Send(1));
}

TEST_F(RoutingNetworkTest, FUNC_SendToNonExistingNode) {
  EXPECT_TRUE(env_->Send(NodeId(NodeId::kRandomId)));
  EXPECT_TRUE(env_->Send(env_->nodes_[env_->RandomNodeIndex()]->node_id()));
}

TEST_F(RoutingNetworkTest, FUNC_ClientSend) {
  EXPECT_TRUE(env_->Send(1));
  Sleep(boost::posix_time::seconds(21));  // This sleep is required for un-responded requests
}

TEST_F(RoutingNetworkTest, FUNC_SendMulti) {
  EXPECT_TRUE(env_->Send(5));
}

TEST_F(RoutingNetworkTest, FUNC_ClientSendMulti) {
  EXPECT_TRUE(env_->Send(3));
// This sleep is required for un-responded requests
  Sleep(boost::posix_time::seconds(static_cast<long>(env_->nodes_.size() + 1)));  // NOLINT (Fraser)
}

TEST_F(RoutingNetworkTest, FUNC_SendToGroup) {
  uint16_t message_count(10), receivers_message_count(0);
  size_t last_index(kServerSize - 1);
  NodeId dest_id(env_->nodes_[last_index]->node_id());

  env_->ClearMessages();
  EXPECT_TRUE(env_->GroupSend(dest_id, message_count));
  for (size_t index = 0; index != (last_index); ++index)
    receivers_message_count += static_cast<uint16_t>(env_->nodes_.at(index)->MessagesSize());

  EXPECT_EQ(0, env_->nodes_[last_index]->MessagesSize())
      << "Not expected message at Node : "
      << HexSubstr(env_->nodes_[last_index]->node_id().string());
  EXPECT_EQ(message_count * (Parameters::node_group_size), receivers_message_count);
}

TEST_F(RoutingNetworkTest, FUNC_SendToGroupSelfId) {
  uint16_t message_count(10), receivers_message_count(0);
  size_t last_index(kServerSize - 1);
  NodeId dest_id(env_->nodes_[0]->node_id());

  env_->ClearMessages();
  EXPECT_TRUE(env_->GroupSend(dest_id, message_count));
  for (size_t index = 0; index != (last_index); ++index)
    receivers_message_count += static_cast<uint16_t>(env_->nodes_.at(index)->MessagesSize());

  EXPECT_EQ(0, env_->nodes_[0]->MessagesSize())
        << "Not expected message at Node : "
        << HexSubstr(env_->nodes_[0]->node_id().string());
  EXPECT_EQ(message_count * (Parameters::node_group_size), receivers_message_count);
}

TEST_F(RoutingNetworkTest, FUNC_SendToGroupClientSelfId) {
  uint16_t message_count(100), receivers_message_count(0);

  size_t client_index(env_->RandomClientIndex());

  size_t last_index(env_->nodes_.size());
  NodeId dest_id(env_->nodes_[client_index]->node_id());

  env_->ClearMessages();
  EXPECT_TRUE(env_->GroupSend(dest_id, message_count, client_index));  // from client
  for (size_t index = 0; index != (last_index); ++index)
    receivers_message_count += static_cast<uint16_t>(env_->nodes_.at(index)->MessagesSize());

  EXPECT_EQ(0, env_->nodes_[client_index]->MessagesSize())
        << "Not expected message at Node : "
        << HexSubstr(env_->nodes_[client_index]->node_id().string());
  EXPECT_EQ(message_count * (Parameters::node_group_size), receivers_message_count);
}

TEST_F(RoutingNetworkTest, FUNC_SendToGroupInHybridNetwork) {
  uint16_t message_count(1), receivers_message_count(0);
  LOG(kVerbose) << "Network created";
  size_t last_index(env_->nodes_.size() - 1);
  NodeId dest_id(env_->nodes_[last_index]->node_id());

  env_->ClearMessages();
  EXPECT_TRUE(env_->GroupSend(dest_id, message_count));
  for (size_t index = 0; index != (last_index); ++index)
    receivers_message_count += static_cast<uint16_t>(env_->nodes_.at(index)->MessagesSize());

  EXPECT_EQ(0, env_->nodes_[last_index]->MessagesSize())
        << "Not expected message at Node : "
        << HexSubstr(env_->nodes_[last_index]->node_id().string());
  EXPECT_EQ(message_count * (Parameters::node_group_size), receivers_message_count);
}

TEST_F(RoutingNetworkTest, FUNC_SendToGroupRandomId) {
  uint16_t message_count(200), receivers_message_count(0);
  env_->ClearMessages();
  for (int index = 0; index < message_count; ++index) {
    EXPECT_TRUE(env_->GroupSend(NodeId(NodeId::kRandomId), 1));
    for (auto node : env_->nodes_) {
      receivers_message_count += static_cast<uint16_t>(node->MessagesSize());
      node->ClearMessages();
    }
  }
  EXPECT_EQ(message_count * (Parameters::node_group_size), receivers_message_count);
  LOG(kVerbose) << "Total message received count : "
                << message_count * (Parameters::node_group_size);
}

TEST_F(RoutingNetworkTest, FUNC_AnonymousSendToGroupRandomId) {
  uint16_t message_count(200), receivers_message_count(0);
  env_->AddNode(true, NodeId(), true);
  assert(env_->nodes_.size() - 1 < std::numeric_limits<uint16_t>::max());
  for (int index = 0; index < message_count; ++index) {
    EXPECT_TRUE(env_->GroupSend(NodeId(NodeId::kRandomId), 1,
                                static_cast<uint16_t>(env_->nodes_.size() - 1)));
    for (auto node : env_->nodes_) {
      receivers_message_count += static_cast<uint16_t>(node->MessagesSize());
      node->ClearMessages();
    }
  }
  EXPECT_EQ(message_count * (Parameters::node_group_size), receivers_message_count);
  LOG(kVerbose) << "Total message received count : "
                << message_count * (Parameters::node_group_size);
}

TEST_F(RoutingNetworkTest, FUNC_AnonymousSendToGroupExistingId) {
  uint16_t message_count(200), receivers_message_count(0);
  size_t initial_network_size(env_->nodes_.size());
  env_->AddNode(true, NodeId(), true);
  assert(env_->nodes_.size() - 1 < std::numeric_limits<uint16_t>::max());
  for (int index = 0; index < message_count; ++index) {
    int group_id_index = index % initial_network_size;  // all other nodes
    NodeId group_id(env_->nodes_[group_id_index]->node_id());
    EXPECT_TRUE(env_->GroupSend(group_id, 1, static_cast<uint16_t>(env_->nodes_.size() - 1)));
    for (auto node : env_->nodes_) {
      receivers_message_count += static_cast<uint16_t>(node->MessagesSize());
      node->ClearMessages();
    }
  }
  EXPECT_EQ(message_count * (Parameters::node_group_size), receivers_message_count);
  LOG(kVerbose) << "Total message received count : "
                << message_count * (Parameters::node_group_size);
}

TEST_F(RoutingNetworkTest, FUNC_JoinAfterBootstrapLeaves) {
  LOG(kVerbose) << "Network Size " << env_->nodes_.size();
  Sleep(boost::posix_time::seconds(10));
  LOG(kVerbose) << "RIse ";
  env_->AddNode(false, NodeId());
//  env_->AddNode(true, NodeId());
}

// This test produces the recursive call.
TEST_F(RoutingNetworkTest, FUNC_RecursiveCall) {
  for (int index(0); index < 8; ++index)
    env_->AddNode(false, GenerateUniqueRandomId(20));
  env_->AddNode(true, GenerateUniqueRandomId(40));
  env_->AddNode(false, GenerateUniqueRandomId(35));
  env_->AddNode(false, GenerateUniqueRandomId(30));
  env_->AddNode(false, GenerateUniqueRandomId(25));
  env_->AddNode(false, GenerateUniqueRandomId(20));
  env_->AddNode(false, GenerateUniqueRandomId(10));
  env_->AddNode(true, GenerateUniqueRandomId(10));
  env_->PrintRoutingTables();
}

TEST_F(RoutingNetworkTest, FUNC_JoinWithSameId) {
  NodeId node_id(NodeId::kRandomId);
  env_->AddNode(true, node_id);
  env_->AddNode(true, node_id);
  env_->AddNode(true, node_id);
  env_->AddNode(true, node_id);
}

TEST_F(RoutingNetworkTest, FUNC_SendToClientsWithSameId) {
  const uint16_t kMessageCount(50);
  NodeId node_id(NodeId::kRandomId);
  for (uint16_t index(0); index < 4; ++index)
    env_->AddNode(true, node_id);

  for (uint16_t index(0); index < kMessageCount; ++index)
    EXPECT_TRUE(env_->Send(env_->nodes_[kNetworkSize],
                           env_->nodes_[kNetworkSize]->node_id(),
                           true));
  uint16_t num_of_tries(0);
  bool done(false);
  do {
    Sleep(boost::posix_time::seconds(1));
    size_t size(0);
    for (auto node : env_->nodes_) {
      size += node->MessagesSize();
    }
    if (4 * kMessageCount == size) {
      done = true;
      num_of_tries = 19;
    }
    ++num_of_tries;
  } while (num_of_tries < 20);
  EXPECT_TRUE(done);  // the number of 20 may need to be increased
}

TEST_F(RoutingNetworkTest, FUNC_SendToClientWithSameId) {
  NodeId node_id(env_->nodes_.at(env_->RandomClientIndex())->node_id());
  uint32_t new_index(env_->nodes_.size());
  env_->AddNode(true, node_id);
  size_t size(0);

  env_->ClearMessages();
  EXPECT_TRUE(env_->Send(env_->nodes_[new_index],
                         node_id,
                         true));
  Sleep(boost::posix_time::seconds(1));
  for (auto node : env_->nodes_) {
    size += node->MessagesSize();
  }
  EXPECT_EQ(2, size);
}

TEST_F(RoutingNetworkTest, FUNC_NodeRemoved) {
  size_t random_index(env_->RandomNodeIndex());
  NodeInfo removed_node_info(env_->nodes_[random_index]->GetRemovableNode());
  EXPECT_GE(removed_node_info.bucket, 510);
}

TEST_F(RoutingNetworkTest, FUNC_GetRandomExistingNode) {
  uint32_t collisions(0);
  uint32_t kChoseIndex(env_->RandomNodeIndex());
  EXPECT_TRUE(env_->Send(1));
//  EXPECT_LT(env_->nodes_[random_node]->RandomNodeVector().size(), 98);
//  for (auto node : env_->nodes_[random_node]->RandomNodeVector())
//    LOG(kVerbose) << HexSubstr(node.string());
  NodeId last_node(NodeId::kRandomId), last_random(NodeId::kRandomId);
  for (auto index(0); index < 100; ++index) {
    last_node = env_->nodes_[kChoseIndex]->GetRandomExistingNode();
    if (last_node == last_random) {
      LOG(kVerbose) << HexSubstr(last_random.string()) << ", " << HexSubstr(last_node.string());
      collisions++;
//      for (auto node : env_->nodes_[random_node]->RandomNodeVector())
//        LOG(kVerbose) << HexSubstr(node.string());
    }
    last_random = last_node;
  }
  ASSERT_LT(collisions, 50);
  for (int i(0); i < 120; ++i)
    env_->nodes_[kChoseIndex]->AddNodeToRandomNodeHelper(NodeId(NodeId::kRandomId));

  // Check there are 100 unique IDs in the RandomNodeHelper
  std::set<NodeId> random_node_ids;
  int attempts(0);
  while (attempts < 10000 && random_node_ids.size() < 100) {
    NodeId retrieved_id(env_->nodes_[kChoseIndex]->GetRandomExistingNode());
    env_->nodes_[kChoseIndex]->RemoveNodeFromRandomNodeHelper(retrieved_id);
    random_node_ids.insert(retrieved_id);
  }
  EXPECT_EQ(100, random_node_ids.size());
}

}  // namespace test

}  // namespace routing

}  // namespace maidsafe
