@echo off
echo ========================================
echo ATLAS SYSTEM TEST SCRIPT
echo ========================================
echo.
echo This will test all Atlas systems via console commands.
echo Make sure the game is running and console is open.
echo.
echo Copy and paste these commands into the Unreal console:
echo.
echo --- BASIC TEST ---
echo Atlas.ShowIntegrityStatus
echo Atlas.Phase3.QuickTest
echo.
echo --- ROOM SYSTEM TEST ---
echo Atlas.Phase3.StartRun
echo Atlas.Phase3.ShowRunProgress
echo Atlas.Phase3.ListRooms
echo.
echo --- REWARD TEST ---
echo Atlas.Phase3.ListRewards
echo Atlas.Phase3.GiveReward Berserker
echo Atlas.Phase3.ShowSlots
echo.
echo --- ROOM PROGRESSION ---
echo Atlas.Phase3.CompleteRoom
echo Atlas.Phase3.ShowRunProgress
echo.
echo --- PHASE 4 TEST ---
echo Atlas.Phase4.TestAll
echo.
echo ========================================
echo If StartRun works, the system is functional!
echo ========================================
pause