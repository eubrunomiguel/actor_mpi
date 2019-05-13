//
// Created by Bruno Macedo Miguel on 2019-05-11.
//

#ifndef ACTORUPCXX_PORT_H
#define ACTORUPCXX_PORT_H

#include "utils/helper.h"
#include "utils/mpi_helper.hpp"
#include <string>

template <typename T> class PortIdentification {
public:
  PortIdentification() = default;

  PortIdentification(const std::string &portName, MPIHelper::RankId rankId)
      : tagIdentification(helper::compute_hash(portName)), portName(portName),
        rankId(rankId), port(nullptr) {}

  explicit PortIdentification(std::shared_ptr<T> port)
      : tagIdentification(0), rankId(MPIHelper::InvalidRankId), port(port) {}

  bool isLocal() const { return port != nullptr; }

  bool isExternal() const { return rankId != MPIHelper::InvalidRankId; }

  bool isConnected() const { return isLocal() || isExternal(); }

  auto getRank() { return rankId; }

  auto getName() { return portName; }

  auto getTag() { return rankId; }

  auto getPort() { return port.get(); }

private:
  const int tagIdentification;
  const std::string portName;
  const MPIHelper::RankId rankId;
  std::shared_ptr<T> port;
};

#endif
