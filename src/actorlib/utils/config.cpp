/**
 * @file
 * This file is part of actorlib.
 *
 * @author Alexander Pöppl (poeppl AT in.tum.de, https://www5.in.tum.de/wiki/index.php/Alexander_P%C3%B6ppl,_M.Sc.)
 *
 * @section LICENSE
 *
 * actorlib is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * actorlib is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with actorlib.  If not, see <http://www.gnu.org/licenses/>.
 *
 *
 * @section DESCRIPTION
 *
 */

#include "config.hpp"

#include <sstream>

#define STR(...) #__VA_ARGS__
#define S(...) STR(__VA_ARGS__)

#ifdef GASNET_PAR
const bool config::isGasnetSequentialBackend = false;
#else
const bool config::isGasnetSequentialBackend = true;
#endif

const std::string config::upcxxCodemode = S(ACTORLIB_UPCXX_CODEMODE);
const std::string config::gasnetConduit = (std::string(S(ACTORLIB_UPCXX_GASNET_CONDUIT)).length() == 0) ? "smp" : S(ACTORLIB_UPCXX_GASNET_CONDUIT);
const std::string config::upcxxInstallation = S(ACTORLIB_UPCXX_INSTALLATION);

const std::string config::gitRevision = S(ACTORLIB_GIT_REVISION);
const std::string config::gitCommitDate = S(ACTORLIB_GIT_DATE);
const std::string config::gitCommitMessage = S(ACTORLIB_GIT_COMMIT_MSG);

std::string config::configToString() {
    std::stringstream ss;
    ss << "UPC++ Actor Library Configuration" << std::endl;
    ss << "=================================" << std::endl;
    ss << "Git revision:         " << config::gitRevision << std::endl;
    ss << "Git revision date:    " << config::gitCommitDate << std::endl;
    ss << "Git revision message: " << config::gitCommitMessage << std::endl;
    ss << "UPC++ install:        " << config::upcxxInstallation << std::endl;
    ss << "UPC++ codemode:       " << config::upcxxCodemode << std::endl;
    ss << "GASNET Conduit:       " << config::gasnetConduit << std::endl;
    ss << "GASNET Backend type:  " << (config::isGasnetSequentialBackend ? "Sequential" : "Parallel") << " Backend" << std::endl;
    return ss.str();
}
