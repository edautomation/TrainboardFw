// Trainboard.ch
// Copyright (C) 2024 Emile Décosterd
//
// This program is free software: you can redistribute it and/or modify
// it under the terms of the GNU General Public License as published by
// the Free Software Foundation, either version 3 of the License, or
// (at your option) any later version.
//
// This program is distributed in the hope that it will be useful,
// but WITHOUT ANY WARRANTY; without even the implied warranty of
// MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
// GNU General Public License for more details.
//
// You should have received a copy of the GNU General Public License
// along with this program.  If not, see <http://www.gnu.org/licenses/>.

#ifndef TRANSITION_LOAD_HISTORY_H_
#define TRANSITION_LOAD_HISTORY_H_

#include "DataManager.h"
#include "Fsm.h"

class TransitionLoadHistory : public FsmTransition
{
  public:
    explicit TransitionLoadHistory(FsmState* const destination)
        : FsmTransition(destination) {}

    void Execute() override
    {
        DataMgr_SetWriterMode(DataWriterMode::kMultiple);
    }
};

#endif  // TRANSITION_LOAD_HISTORY_H_
