/*******************************************************************************
 * Copyright (C) Gallium Studio LLC. All rights reserved.
 *
 * This program is open source software: you can redistribute it and/or
 * modify it under the terms of the GNU General Public License as published
 * by the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 * 
 * Alternatively, this program may be distributed and modified under the
 * terms of Gallium Studio LLC commercial licenses, which expressly supersede
 * the GNU General Public License and are specifically designed for licensees
 * interested in retaining the proprietary status of their code.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program. If not, see <http://www.gnu.org/licenses/>.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
 * OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 * 
 * Contact information:
 * Website - https://www.galliumstudio.com
 * Source repository - https://github.com/galliumstudio
 * Email - admin@galliumstudio.com
 ******************************************************************************/

#ifndef ELEVATOR_INTERFACE_H
#define ELEVATOR_INTERFACE_H

#include "fw_def.h"
#include "fw_evt.h"
#include "app_hsmn.h"

using namespace QP;
using namespace FW;

namespace APP {

#define ELEVATOR_INTERFACE_EVT \
    ADD_EVT(ELEVATOR_START_REQ) \
    ADD_EVT(ELEVATOR_START_CFM) \
    ADD_EVT(ELEVATOR_STOP_REQ) \
    ADD_EVT(ELEVATOR_STOP_CFM) \
    ADD_EVT(ELEVATOR_ERROR_REQ) \
    ADD_EVT(ELEVATOR_MOVE_REQ) \
    ADD_EVT(ELEVATOR_MOVE_CFM) \
    ADD_EVT(ELEVATOR_MOVE_UP_ONE_REQ) \
    ADD_EVT(ELEVATOR_MOVE_DOWN_ONE_REQ) \
    ADD_EVT(ELEVATOR_IDLE_REQ) \


#undef ADD_EVT
#define ADD_EVT(e_) e_,

enum {
    ELEVATOR_INTERFACE_EVT_START = INTERFACE_EVT_START(ELEVATOR),
    ELEVATOR_INTERFACE_EVT
};

enum {
    ELEVATOR_REASON_UNSPEC = 0,
};

class ElevatorStartReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    ElevatorStartReq() :
        Evt(ELEVATOR_START_REQ) {}
};

class ElevatorStartCfm : public ErrorEvt {
public:
    ElevatorStartCfm(Error error, Hsmn origin = HSM_UNDEF, Reason reason = 0) :
        ErrorEvt(ELEVATOR_START_CFM, error, origin, reason) {}
};

class ElevatorStopReq : public Evt {
public:
    enum {
        TIMEOUT_MS = 100
    };
    ElevatorStopReq() :
        Evt(ELEVATOR_STOP_REQ) {}
};

class ElevatorStopCfm : public ErrorEvt {
public:
    ElevatorStopCfm(Error error, Hsmn origin = HSM_UNDEF, Reason reason = 0) :
        ErrorEvt(ELEVATOR_STOP_CFM, error, origin, reason) {}
};

// There is no accompanying CFM.
class ElevatorErrorReq : public Evt {
public:
    ElevatorErrorReq() :
        Evt(ELEVATOR_ERROR_REQ) {}
};

class ElevatorMoveReq : public Evt {
public:
    ElevatorMoveReq(uint32_t floorReq, bool isInsideElevator = false) :
        Evt(ELEVATOR_MOVE_REQ), m_floorReq(floorReq), m_isInsideElevator(isInsideElevator) {}
    uint32_t GetFloorRequested() const { return m_floorReq; }
    bool IsInsideElevator() const { return m_isInsideElevator; }
private:
    uint32_t m_floorReq;
    bool m_isInsideElevator;
};

class ElevatorMoveCfm : public ErrorEvt {
public:
    ElevatorMoveCfm(Error error, Hsmn origin = HSM_UNDEF, Reason reason = 0) :
        ErrorEvt(ELEVATOR_MOVE_CFM, error, origin, reason) {}
};

// There is no accompanying CFM.
class ElevatorMoveUpOneReq : public Evt {
public:
    ElevatorMoveUpOneReq() :
        Evt(ELEVATOR_MOVE_UP_ONE_REQ){}
};

// There is no accompanying CFM.
class ElevatorMoveDownOneReq : public Evt {
public:
    ElevatorMoveDownOneReq() :
        Evt(ELEVATOR_MOVE_DOWN_ONE_REQ){}
};

// There is no accompanying CFM.
class ElevatorIdleReq : public Evt {
public:
    ElevatorIdleReq() :
        Evt(ELEVATOR_IDLE_REQ) {}
};

} // namespace APP

#endif // ELEVATOR_INTERFACE_H
