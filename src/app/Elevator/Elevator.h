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

#ifndef ELEVATOR_H
#define ELEVATOR_H

#include "qpcpp.h"
#include "fw_active.h"
#include "fw_timer.h"
#include "fw_evt.h"
#include "app_hsmn.h"
#include "Lamp.h"

using namespace QP;
using namespace FW;

namespace APP {

class Elevator : public Active {
public:
    Elevator();

protected:
    static QState InitialPseudoState(Elevator * const me, QEvt const * const e);
    static QState Root(Elevator * const me, QEvt const * const e);
        static QState Stopped(Elevator * const me, QEvt const * const e);
        static QState Started(Elevator * const me, QEvt const * const e);
           static QState MovingUp(Elevator * const me, QEvt const * const e);
           static QState MovingDown(Elevator * const me, QEvt const * const e);
           static QState DoorOpened(Elevator * const me, QEvt const * const e);
           static QState DoorClosed(Elevator * const me, QEvt const * const e);
           static QState Idle(Elevator * const me, QEvt const * const e);
           static QState ErrorState(Elevator * const me, QEvt const * const e);

    enum {
    	DOOR_WAIT_TIMEOUT_MS = 2000,
    	FlOOR_WAIT_TIMEOUT_MS = 1000
    };
    Timer m_waitTimer;         // Timer used to wait for the minimum wait duration or the yellow light (slow-down) duration in either direction.
    uint32_t m_currentFloor;   // used to tell what floor the elevator is currently on.
    uint32_t m_requestedFloor; // used to tell what floor is currently requested.
    bool m_isDoorOpen;         // used to tell if the elevator door is currently open.

    const uint32_t floorY[5] = {250, 190, 130, 70, 10};  // Y values for drawing the elevator location based on floor.

#define ELEVATOR_TIMER_EVT \
    ADD_EVT(WAIT_TIMER) \

// Placeholder only.
#define ELEVATOR_INTERNAL_EVT \
    ADD_EVT(DONE) \
    ADD_EVT(FAILED)

#undef ADD_EVT
#define ADD_EVT(e_) e_,

    enum {
        ELEVATOR_TIMER_EVT_START = TIMER_EVT_START(ELEVATOR),
        ELEVATOR_TIMER_EVT
    };

    enum {
        ELEVATOR_INTERNAL_EVT_START = INTERNAL_EVT_START(ELEVATOR),
        ELEVATOR_INTERNAL_EVT
    };

    // Placeholder only.
    class Failed : public ErrorEvt {
    public:
        Failed(Hsmn hsmn, Error error, Hsmn origin, Reason reason) :
            ErrorEvt(FAILED, hsmn, hsmn, 0, error, origin, reason) {}
    };
};

} // namespace APP

#endif // ELEVATOR_H
