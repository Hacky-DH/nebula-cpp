/* Copyright (c) 2018 vesoft inc. All rights reserved.
 *
 * This source code is licensed under Apache 2.0 License.
 */

#pragma once

#include <memory>
#include <string>
#include <thread>
#include <unordered_map>
#include <vector>

#include "ScanEdgeIter.h"
#include "common/datatypes/HostAddr.h"
#include "common/thrift/ThriftTypes.h"
#include "nebula/mclient/MetaClient.h"
#include "nebula/sclient/SConfig.h"
#include "nebula/sclient/ScanEdgeIter.h"

namespace folly {
class IOThreadPoolExecutor;
template <class T>
class Promise;
}  // namespace folly

namespace nebula {

namespace thrift {

template <class ClientType>
class ThriftClientManager;

}  // namespace thrift

namespace meta {
namespace cpp2 {

class MetaServiceAsyncClient;

}  // namespace cpp2
}  // namespace meta

namespace storage {
namespace cpp2 {

class GraphStorageServiceAsyncClient;
class ScanCursor;
class ScanEdgeRequest;
class ScanResponse;

}  // namespace cpp2
}  // namespace storage

#define DEFAULT_LIMIT std::numeric_limits<int64_t>::max()
#define DEFAULT_START_TIME 0
#define DEFAULT_END_TIME std::numeric_limits<int64_t>::max()

class StorageClient {
  friend struct ScanEdgeIter;

 public:
  explicit StorageClient(const std::vector<std::string>& metaAddrs,
                         const MConfig& mConfig = MConfig{},
                         const SConfig& sConfig = SConfig{});

  ~StorageClient();

  std::vector<PartitionID> getParts(const std::string& spaceName);  // plato needed

  ScanEdgeIter scanEdgeWithPart(std::string spaceName,
                                int32_t partID,
                                std::string edgeName,
                                std::vector<std::string> propNames,
                                int64_t limit = DEFAULT_LIMIT,
                                int64_t startTime = DEFAULT_START_TIME,
                                int64_t endTime = DEFAULT_END_TIME,
                                std::string filter = "",
                                bool onlyLatestVersion = false,
                                bool enableReadFromFollower = true);  // plato needed

  MetaClient* getMetaClient() {
    return mClient_.get();
  }

 private:
  std::pair<bool, storage::cpp2::ScanResponse> doScanEdge(
      const storage::cpp2::ScanEdgeRequest& req);

  template <typename Request, typename RemoteFunc, typename Response>
  void getResponse(std::pair<HostAddr, Request>&& request,
                   RemoteFunc&& remoteFunc,
                   folly::Promise<std::pair<bool, Response>> pro);

  std::unique_ptr<MetaClient> mClient_;
  SConfig sConfig_;
  std::shared_ptr<folly::IOThreadPoolExecutor> ioExecutor_;
  std::shared_ptr<thrift::ThriftClientManager<storage::cpp2::GraphStorageServiceAsyncClient>>
      clientsMan_;
};

}  // namespace nebula
