# Atlas Quick Reference

## ✅ Current State
- **Architecture**: Fully unified ActionManagerComponent system
- **Legacy Code**: All removed (CombatComponent, DamageCalculator, old abilities)
- **Interfaces**: 4 clean interfaces (Combat, Health, Action, Interactable)
- **Combat**: Animation-driven with notifies
- **Data**: Everything configurable via ActionDataAssets

## 🎮 What Works
- Basic/Heavy attacks with animation notifies
- Block system (40% damage reduction)
- Vulnerability (8x damage multiplier)
- Poise/Stagger system
- 15 abilities (need DataAsset configuration)
- Console commands for slot management

## ⚠️ TODO
1. **Configure DataAssets** in editor for all 15 abilities
2. **Assign Montages** to each ability
3. **Test** each ability systematically
4. **Enemy AI** implementation (P17-18)
5. **Wife's Arm** passive system (P21-22)

## 🔧 Key Files
- `ActionManagerComponent.cpp` - All combat/action logic
- `UniversalAction.cpp` - Action execution
- `ActionDataAsset.h` - Configuration structure
- Animation Notifies in `AnimationNotifies/` folder

## 📝 Remember
- Set `bCanBeInterrupted = false` for attack DataAssets
- Use animation notifies for all timing
- All components communicate via interfaces
- No GAS, no parry, no camera lock