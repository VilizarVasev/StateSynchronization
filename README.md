# Device State Synchronization with FreeRTOS

This project simulates two devices as separate FreeRTOS tasks on the Windows
FreeRTOS simulator. DeviceA is the master and synchronizes its state with the
state-change events produced by DeviceB.

## Architecture

The application is split into three components:

- `DeviceA`: Implements the DeviceA state machine and persistent-fault logic.
- `DeviceB`: Implements the autonomous DeviceB state machine and reset logic.
- `Common`: Owns the two FreeRTOS queues and defines the messages shared by the
  devices.

Each device task has a minimal infinite loop that calls its state evaluator.
The evaluator switches on the component's private `currentState` and dispatches
to one handler per state. Both evaluators include an invalid-state `default`
handler.

```c
for( ; ; )
{
    DeviceX_EvaluateState();
}
```

The communication is the following:

```text
DeviceA -- DeviceBCommand_t -----------> DeviceB reset queue ------> DeviceB
DeviceA <- DeviceBState_t <------------- event queue <-------------- DeviceB
```

The reset queue contains one command because only the latest reset
is relevant. The event queue also contains one entry and provides the 
latest DeviceB state. DeviceB overwrites an older unprocessed state 
whenever it publishes a new state. 

## State synchronization

DeviceB has the states `SLEEP`, `ACTIVE`, and `FAULT`. Every 2000 ms, while it
is not in `FAULT`, it uses the standard C expression `rand() % 100` to produce
a roll from 0 to 99. In `SLEEP`, rolls below 40 select `ACTIVE` and rolls of 90
or greater select `FAULT`. In `ACTIVE`, rolls below 30 select `SLEEP` and rolls
of 80 or greater select `FAULT`. Other rolls preserve the current state.

After every actual transition, DeviceB overwrites the state in the event queue.
DeviceA waits on that queue for up to 1000 ms and applies this mapping when an
event arrives:

| DeviceB state | DeviceA state |
|---|---|
| `SLEEP` | `IDLE` |
| `ACTIVE` | `PROCESSING` |
| `FAULT` | `ERROR` |

DeviceA stores the last DeviceB state received from the queue in a private
component variable. If the queue receive times out, that variable is unchanged,
so each handler continues to evaluate the last known DeviceB state.

In `FAULT`, DeviceB waits up to 2000 ms for a reset command. If reset command 
does not arrive, a random roll below 20 recovers it to `SLEEP`, 
a roll from 20 through 39 recovers it to `ACTIVE`, and any other roll 
keeps it in `FAULT`. 
A recovery transition is published immediately. 
DeviceA waits up to 1000 ms for each event and increments
its fault counter while its last known DeviceB state remains `FAULT`. After three
evaluations it sends a reset command.


## Build and run

- Clone the FreeRTOS Windows/Linux simulationrepository.
- Compile and test the repository.
- Paste the task solution in the repository, e. g.
<< Path to FreeRTOS repository>>\Projects\StateSynchronization

From PowerShell:

```powershell
cd <<Path to FreeRTOS repository>>\Projects\StateSynchronization
.\build.cmd
.\run.cmd
```

`build.cmd` uses the repository-local GCC, CMake, and Ninja tools. `run.cmd`
builds the application and starts `build\state_sync_simulation.exe`. Stop the
simulation with `Ctrl+C` or by closing its console.

## Expected simulation behavior

The log contains plain-text state transitions, fault detection, reset commands,
and recovery. A normal run shows:

1. Both devices starting in `IDLE`/`SLEEP`.
2. DeviceB autonomously entering `ACTIVE`; DeviceA enters `PROCESSING`.
3. DeviceB autonomously entering `FAULT`; DeviceA enters `ERROR`.
4. DeviceB recovering naturally to `SLEEP` or `ACTIVE`, or remaining in `FAULT`.
5. If the fault persists, DeviceA sends a reset and DeviceB returns to `SLEEP`.

## Potential extensions

- Make timing, queue lengths, and the pseudo-random seed runtime-configurable.
- Add queue-send failure counters and high-water-mark instrumentation.
- Replace dynamic queue/task allocation with static allocation for an embedded
  target.
- Add unit tests for each transition table and integration tests for queue
  overflow and delayed consumers.
- Replace the deterministic simulation PRNG seed with a hardware entropy source
  on a physical target.
