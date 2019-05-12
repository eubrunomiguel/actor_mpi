//
// Created by Bruno Macedo Miguel on 2019-05-11.
//

#ifndef ACTORUPCXX_PORT_H
#define ACTORUPCXX_PORT_H

#include "utils/helper.h"
#include "utils/mpi_helper.hpp"
#include <string>

template <typename T> struct PortIdentification {
  PortIdentification() = default;

  PortIdentification(const std::string &portName, MPIHelper::RankId rankId)
      : tagIdentification(helper::compute_hash(portName)), portName(portName),
        rankId(rankId), port(nullptr) {}

  explicit PortIdentification(T *port)
      : tagIdentification(0), rankId(MPIHelper::InvalidRankId), port(port) {}

  bool isLocal() const { return port != nullptr; }

  bool isExternal() const { return rankId != MPIHelper::InvalidRankId; }

  bool isConnected() const { return isLocal() || isExternal(); }

  const int tagIdentification;
  const std::string portName;
  const MPIHelper::RankId rankId;
  T *const port;
};

#endif // ACTORUPCXX_PORT_H
