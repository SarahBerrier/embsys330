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
    m_lampNS(LAMP_NS, "LAMP_NS"), m_lampEW(LAMP_EW, "LAMP_EW"),
    m_carWaiting(false), m_waitTimer(GetHsmn(), WAIT_TIMER),
    m_idleTimer(GetHsmn(), IDLE_TIMER), m_blinkTimer(GetHsmn(), BLINK_TIMER){
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
            // Initialize regions.
            me->m_lampNS.Init(me);
            me->m_lampEW.Init(me);
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
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            return Q_TRAN(&Elevator::NSGo);
        }
        case ELEVATOR_STOP_REQ: {
            EVENT(e);
            Evt const &req = EVT_CAST(*e);

            // @todo Need to wait for response.
            me->Send(new LampResetReq(), LAMP_NS);
            me->Send(new LampResetReq(), LAMP_EW);

            // @todo Need to wait for response.
            me->Send(new DispStopReq(), ILI9341);
            me->SendCfm(new ElevatorStopCfm(ERROR_SUCCESS), req);
            return Q_TRAN(&Elevator::Stopped);
        }
        case ELEVATOR_ERROR_REQ: {
            EVENT(e);
            return Q_TRAN(&Elevator::StopSign);
        }
    }
    return Q_SUPER(&Elevator::Root);
}

QState Elevator::NSGo(Elevator *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->Send(new LampRedReq(), LAMP_EW);
            me->Send(new LampGreenReq(), LAMP_NS);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            return Q_TRAN(&Elevator::NSMinTimeWait);
        }
        case ELEVATOR_CAR_EW_REQ: {
            EVENT(e);
            return Q_TRAN(&Elevator::NSSlow);
        }
    }
    return Q_SUPER(&Elevator::Started);;
}

QState Elevator::NSMinTimeWait(Elevator *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_carWaiting = false;
            me->m_waitTimer.Start(NS_MIN_WAIT_TIMEOUT_MS);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            if (me->m_carWaiting) {
                me->Send(new ElevatorCarEWReq(), me->GetHsmn());
            }
            me->m_waitTimer.Stop();
            return Q_HANDLED();
        }
        case ELEVATOR_CAR_EW_REQ: {
            EVENT(e);
            me->m_carWaiting = true;
            return Q_HANDLED();
        }
        case WAIT_TIMER: {
            EVENT(e);
            return Q_TRAN(&Elevator::NSMinTimeExceeded);
        }
    }
    return Q_SUPER(&Elevator::NSGo);;
}

QState Elevator::NSMinTimeExceeded(Elevator *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Elevator::NSGo);;
}

QState Elevator::NSSlow(Elevator *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->Send(new LampYellowReq(), LAMP_NS);
            me->m_waitTimer.Start(NS_SLOW_TIMEOUT_MS);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            me->m_waitTimer.Stop();
            return Q_HANDLED();
        }
        case WAIT_TIMER: {
            EVENT(e);
            return Q_TRAN(&Elevator::EWGo);
        }
    }
    return Q_SUPER(&Elevator::Started);
}

QState Elevator::EWGo(Elevator *me, QEvt const *e) {
    switch (e->sig)
    {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->Send(new LampRedReq(), LAMP_NS);
            me->Send(new LampGreenReq(), LAMP_EW);
            me->m_idleTimer.Start(EW_IDLE_TIMEOUT_MS);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            me->m_idleTimer.Stop();
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            return Q_TRAN(&Elevator::EWMinTimeWait);
        }
        case ELEVATOR_CAR_EW_REQ: {
            EVENT(e);
            me->m_idleTimer.Restart(EW_IDLE_TIMEOUT_MS);
            return Q_HANDLED();
        }
        case ELEVATOR_CAR_NS_REQ:
        case IDLE_TIMER: {
            EVENT(e);
            return Q_TRAN(&Elevator::EWSlow);
        }
    }
    return Q_SUPER(&Elevator::Started);
}

QState Elevator::EWMinTimeWait(Elevator *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_carWaiting = false;
            me->m_waitTimer.Start(EW_MIN_WAIT_TIMEOUT_MS);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            if (me->m_carWaiting) {
                me->Send(new ElevatorCarNSReq(), me->GetHsmn());
            }
            me->m_waitTimer.Stop();
            return Q_HANDLED();
        }
        case ELEVATOR_CAR_NS_REQ: {
            EVENT(e);
            me->m_carWaiting = true;
            return Q_HANDLED();
        }
        case WAIT_TIMER: {
            EVENT(e);
            return Q_TRAN(&Elevator::EWMinTimeExceeded);
        }
    }
    return Q_SUPER(&Elevator::EWGo);
}

QState Elevator::EWMinTimeExceeded(Elevator *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
    }
    return Q_SUPER(&Elevator::EWGo);
}

QState Elevator::EWSlow(Elevator *me, QEvt const *e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->Send(new LampYellowReq(), LAMP_EW);
            me->m_waitTimer.Start(EW_SLOW_TIMEOUT_MS);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            me->m_waitTimer.disarm();
            return Q_HANDLED();
        }
        case WAIT_TIMER: {
            EVENT(e);
            return Q_TRAN(&Elevator::NSGo);
        }
    }
    return Q_SUPER(&Elevator::Started);
}

QState Elevator::StopSign(Elevator * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->m_blinkTimer.Start(BLINK_TIMEOUT_MS, Timer::PERIODIC);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            me->m_blinkTimer.Stop();
            return Q_HANDLED();
        }
        case Q_INIT_SIG: {
            EVENT(e);
            return Q_TRAN(&Elevator::StopSignOn);
        }
    }
    return Q_SUPER(&Elevator::Started);
}

QState Elevator::StopSignOn(Elevator * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->Send(new LampRedReq(), LAMP_NS);
            me->Send(new LampRedReq(), LAMP_EW);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case BLINK_TIMER: {
            EVENT(e);
            return Q_TRAN(&Elevator::StopSignOff);
        }
    }
    return Q_SUPER(&Elevator::StopSign);
}

QState Elevator::StopSignOff(Elevator * const me, QEvt const * const e) {
    switch (e->sig) {
        case Q_ENTRY_SIG: {
            EVENT(e);
            me->Send(new LampOffReq(), LAMP_NS);
            me->Send(new LampOffReq(), LAMP_EW);
            return Q_HANDLED();
        }
        case Q_EXIT_SIG: {
            EVENT(e);
            return Q_HANDLED();
        }
        case BLINK_TIMER: {
            EVENT(e);
            return Q_TRAN(&Elevator::StopSignOn);
        }
    }
    return Q_SUPER(&Elevator::StopSign);;
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
