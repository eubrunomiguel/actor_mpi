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
  PortIdentification() = delete;

  template <class str>
  PortIdentification(str &&portName, mpi::rank rankId)
      : portName(std::forward<str>(portName)),
        tagIdentification(helper::compute_hash(portName)), rankId(rankId),
        port(nullptr) {}

  explicit PortIdentification(T *port)
      : tagIdentification(0), rankId(mpi::INVALID_RANK_ID), port(port) {}

  inline bool isLocal() const { return port != nullptr; }

  inline bool isExternal() const { return rankId != mpi::INVALID_RANK_ID; }

  inline bool isConnected() const { return isLocal() || isExternal(); }

  inline auto getRank() const { return rankId; }

  inline auto getName() const { return portName; }

  inline auto getTag() const { return rankId; }

  inline auto getPort() const { return port; }

private:
  std::string portName;
  int tagIdentification;
  mpi::rank rankId;
  T *port;
};

#endif
