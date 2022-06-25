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

#include "app_hsmn.h"
#include "fw_log.h"
#include "fw_assert.h"
#include "DispInterface.h"
#include "ElevatorInterface.h"
#include "LampInterface.h"
#include "Elevator.h"

FW_DEFINE_THIS_FILE("Elevator.cpp")

namespace APP {

#undef ADD_EVT
#define ADD_EVT(e_) #e_,

static char const * const timerEvtName[] = {
    "ELEVATOR_TIMER_EVT_START",
    ELEVATOR_TIMER_EVT
};

static char const * const internalEvtName[] = {
    "ELEVATOR_INTERNAL_EVT_START",
    ELEVATOR_INTERNAL_EVT
};

static char const * const interfaceEvtName[] = {
    "ELEVATOR_INTERFACE_EVT_START",
    ELEVATOR_INTERFACE_EVT
};

Elevator::Elevator() :
    Active((QStateHandler)&Elevator::InitialPseudoState, ELEVATOR, "ELEVATOR"),
	m_waitTimer(GetHsmn(), WAIT_TIMER), m_currentFloor(1),
	m_requestedFloor(1), m_isDoorOpen(false){
    SET_EVT_NAME(ELEVATOR);
}

QState Elevator::InitialPseudoState(Elevator * const me, QEvt const * const e) {
    (void)e;
    return Q_TRAN(&Elevator::Root);
}

QState Elevator::Root(Elevator * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            EVENT(e);
            return Q_TRAN(&Elevator::Stopped);
        }
        case ELEVATOR_START_REQ: {
            EVENT(e);
            Evt const &req = EVT_CAST(*e);
            me->SendCfm(new ElevatorStartCfm(ERROR_STATE), req);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&QHsm::top);
}

QState Elevator::Stopped(Elevator * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case ELEVATOR_STOP_REQ: {
            EVENT(e);
            Evt const &req = EVT_CAST(*e);
            me->SendCfm(new ElevatorStopCfm(ERROR_SUCCESS), req);
            return Q_HANDLED();
        }
        case ELEVATOR_START_REQ: {
            EVENT(e);
            Evt const &req = EVT_CAST(*e);

            // @todo Need to wait for response.
            me->Send(new DispStartReq(), ILI9341);
            me->Send(new DispDrawBeginReq(), ILI9341);

            me->SendCfm(new ElevatorStartCfm(ERROR_SUCCESS), req);
            return Q_TRAN(&Elevator::Started);
        }
    }
    return Q_SUPER(&Elevator::Root);
}

QState Elevator::Started(Elevator * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            me->Send(new DispDrawRectReq(10, me->floorY[me->m_currentFloor-1], 40, 40, COLOR24_PURPLE), ILI9341);

            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            return Q_TRAN(&Elevator::Idle);
        }
        case ELEVATOR_MOVE_REQ: {
            ElevatorMoveReq const &req = static_cast<ElevatorMoveReq const &>(*e);
            me->m_requestedFloor = req.GetFloorRequested();
			req.IsInsideElevator(); // Sarah - use this?

			if(me->m_requestedFloor == me->m_currentFloor){
				return Q_TRAN(&Elevator::DoorOpened);
			} else if (me->m_requestedFloor > me->m_currentFloor){
				return Q_TRAN(&Elevator::MovingUp);
			} else {
				return Q_TRAN(&Elevator::MovingDown);
			}
        }
        case ELEVATOR_STOP_REQ: {
            EVENT(e);
            Evt const &req = EVT_CAST(*e);

            // @todo Need to wait for response.
            me->Send(new DispStopReq(), ILI9341);
            me->SendCfm(new ElevatorStopCfm(ERROR_SUCCESS), req);
            return Q_TRAN(&Elevator::Stopped);
        }
        case ELEVATOR_ERROR_REQ: {
            EVENT(e);
            return Q_HANDLED();
            //return Q_TRAN(&Elevator::StopSign); sarah to do something better here
        }
    }
    return Q_SUPER(&Elevator::Root);
}

QState Elevator::MovingUp(Elevator *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_waitTimer.Start(FlOOR_WAIT_TIMEOUT_MS);
            return Q_HANDLED();
       }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case WAIT_TIMER: {
            EVENT(e);
         	if (me->m_requestedFloor == me->m_currentFloor){
                LOG("requested floor = %d, current floor = %d", me->m_requestedFloor, me->m_currentFloor);
                return Q_TRAN(&Elevator::DoorOpened);
         	}
         	else {
                me->Raise(new Evt(ELEVATOR_MOVE_UP_ONE_REQ));
         	}
            return Q_HANDLED();
        }
        case ELEVATOR_MOVE_UP_ONE_REQ: {
            EVENT(e);
            LOG("requested floor = %d, current floor = %d", me->m_requestedFloor, me->m_currentFloor);

            me->m_currentFloor++;
            me->Send(new DispDrawRectReq(0, 0, 60, 400, COLOR24_WHITE), ILI9341);
            me->Send(new DispDrawRectReq(10, me->floorY[me->m_currentFloor-1], 40, 40, COLOR24_PURPLE), ILI9341);

            me->m_waitTimer.Start(FlOOR_WAIT_TIMEOUT_MS);

     		return Q_HANDLED();
        }
    }
    return Q_SUPER(&Elevator::Started);
}

QState Elevator::MovingDown(Elevator *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_waitTimer.Start(FlOOR_WAIT_TIMEOUT_MS);
            return Q_HANDLED();
       }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case WAIT_TIMER: {
            EVENT(e);
         	if (me->m_requestedFloor == me->m_currentFloor){
                LOG("requested floor = %d, current floor = %d", me->m_requestedFloor, me->m_currentFloor);
                return Q_TRAN(&Elevator::DoorOpened);
         	}
            else {
                me->Raise(new Evt(ELEVATOR_MOVE_DOWN_ONE_REQ));
            }
            return Q_HANDLED();
        }
        case ELEVATOR_MOVE_DOWN_ONE_REQ: {
            EVENT(e);
            LOG("requested floor = %d, current floor = %d", me->m_requestedFloor, me->m_currentFloor);

            me->m_currentFloor--;
            me->Send(new DispDrawRectReq(0, 0, 60, 400, COLOR24_WHITE), ILI9341);
            me->Send(new DispDrawRectReq(10, me->floorY[me->m_currentFloor-1], 40, 40, COLOR24_PURPLE), ILI9341);

            me->m_waitTimer.Start(FlOOR_WAIT_TIMEOUT_MS);

            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Elevator::Started);
}

QState Elevator::DoorOpened(Elevator *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);

            me->m_isDoorOpen = true;
            me->Send(new DispDrawTextReq("Doors Open", 75, 135, COLOR24_BLACK, COLOR24_WHITE, 2), ILI9341);
            me->m_waitTimer.Start(DOOR_WAIT_TIMEOUT_MS);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            me->m_waitTimer.Stop();
            return Q_HANDLED();
        }
        case WAIT_TIMER: {
            EVENT(e);
            return Q_TRAN(&Elevator::DoorClosed);
        }
    }
    return Q_SUPER(&Elevator::Started);
}

QState Elevator::DoorClosed(Elevator *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);

            me->m_isDoorOpen = false;
            me->Send(new DispDrawTextReq("Doors Closed", 75, 135, COLOR24_BLACK, COLOR24_WHITE, 2), ILI9341);
            me->m_waitTimer.Start(DOOR_WAIT_TIMEOUT_MS);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            me->m_waitTimer.Stop();
            return Q_HANDLED();
        }
        case WAIT_TIMER: {
            EVENT(e);
            return Q_TRAN(&Elevator::Idle);
        }
    }
    return Q_SUPER(&Elevator::Started);
}

QState Elevator::Idle(Elevator *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);

            // draw something Sarah
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        // sarah - do I need something else here?
    }
    return Q_SUPER(&Elevator::Started);
}

/*
QState Elevator::MyState(Elevator * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            return Q_TRAN(&Elevator::SubState);
        }
    }
    return Q_SUPER(&Elevator::SuperState);
}
*/

} // namespace APP
