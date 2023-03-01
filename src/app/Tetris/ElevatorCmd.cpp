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

#include <string.h>
#include "fw_log.h"
#include "fw_assert.h"
#include "Console.h"
#include "ElevatorCmd.h"
#include "ElevatorInterface.h"

FW_DEFINE_THIS_FILE("ElevatorCmd.cpp")

namespace APP {

static bool IsInsideElevator(Console &console, Evt const *e);

static CmdStatus First(Console &console, Evt const *e) {
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            console.PutStr("Elevator request for first floor\n\r");
            bool isInsideElevator = IsInsideElevator(console, e);
            console.Send(new ElevatorMoveReq(1, isInsideElevator), ELEVATOR);
            break;
        }
    }
    return CMD_DONE;
}

static CmdStatus Second(Console &console, Evt const *e) {
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            console.PutStr("Elevator request for second floor\n\r");
            bool isInsideElevator = IsInsideElevator(console, e);
            console.Send(new ElevatorMoveReq(2, isInsideElevator), ELEVATOR);
            break;
        }
    }
    return CMD_DONE;
}

static CmdStatus Third(Console &console, Evt const *e) {
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            console.PutStr("Elevator request for third floor\n\r");
            bool isInsideElevator = IsInsideElevator(console, e);
            console.Send(new ElevatorMoveReq(3, isInsideElevator), ELEVATOR);
            break;
        }
    }
    return CMD_DONE;
}

static CmdStatus Fourth(Console &console, Evt const *e) {
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            console.PutStr("Elevator request for fourth floor\n\r");
            bool isInsideElevator = IsInsideElevator(console, e);
            console.Send(new ElevatorMoveReq(4, isInsideElevator), ELEVATOR);
            break;
        }
    }
    return CMD_DONE;
}

static CmdStatus Fifth(Console &console, Evt const *e) {
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            console.PutStr("Elevator request for fifth floor\n\r");
            bool isInsideElevator = IsInsideElevator(console, e);
            console.Send(new ElevatorMoveReq(5, isInsideElevator), ELEVATOR);
            break;
        }
    }
    return CMD_DONE;
}
static CmdStatus ErrorReq(Console &console, Evt const *e) {
    switch (e->sig) {
        case Console::CONSOLE_CMD: {
            console.PutStr("Critical error occurred\n\r");
            console.Send(new ElevatorErrorReq(), ELEVATOR);
            break;
        }
    }
    return CMD_DONE;
}

static CmdStatus List(Console &console, Evt const *e);
static CmdHandler const cmdHandler[] = {
    { "1",       First,       "First", 0 },
    { "2",       Second,      "Second", 0 },
    { "3",       Third,       "Third", 0 },
    { "4",       Fourth,      "Fourth", 0 },
    { "5",       Fifth,       "Fifth", 0 },
    { "e",       ErrorReq,    "Error", 0 },
    { "?",       List,        "List commands", 0 },
};

static CmdStatus List(Console &console, Evt const *e) {
    return console.ListCmd(e, cmdHandler, ARRAY_COUNT(cmdHandler));
}

// todo sarah - leaving this in for now but I don't think it is needed
static bool IsInsideElevator(Console &console, Evt const *e) {
    bool isInsideElevator = false;
    Console::ConsoleCmd const &ind = static_cast<Console::ConsoleCmd const &>(*e);
    if (ind.Argc() >= 2 && STRING_EQUAL(ind.Argv(1), "1")) {
        isInsideElevator = true;
    }
    return isInsideElevator;
}

CmdStatus ElevatorCmd(Console &console, Evt const *e) {
    return console.HandleCmd(e, cmdHandler, ARRAY_COUNT(cmdHandler));
}

}
