# Atlas Component Architecture

## Component Dependency Graph

```
┌─────────────────────────────────────────────────────────────┐
│                     GameCharacterBase                       │
│                         (Actor)                             │
└──────────────────────┬──────────────────────────────────────┘
                       │
        ┌──────────────┼──────────────┬───────────────────┐
        │              │              │                   │
        ▼              ▼              ▼                   ▼
┌──────────────┐ ┌──────────────┐ ┌──────────────┐ ┌──────────────┐
│HealthComp   │ │ CombatComp   │ │ DashComp     │ │AnimationMgr  │
│              │ │              │ │              │ │              │
│ - Poise      │ │ - Attack     │ │ - Movement   │ │ - Montages   │
│ - Stagger    │ │ - Block      │ │ - IFrames    │ │ - Reactions  │
└──────┬───────┘ └──────┬───────┘ └──────────────┘ └──────────────┘
       │                │
       │                ▼
       │         ┌──────────────┐
       │         │DamageCalc    │
       │         │              │
       │         │ - Processing │
       │         └──────┬───────┘
       │                │
       └────────────────┤
                       │
                       ▼
               ┌──────────────┐
               │Vulnerability │
               │              │
               │ - Multiplier │
               └──────────────┘

Additional Components (Context-Dependent):
┌──────────────┐ ┌──────────────┐ ┌──────────────┐ ┌──────────────┐
│FocusMode     │ │WallImpact    │ │DebugRender   │ │HitboxComp    │
└──────────────┘ └──────────────┘ └──────────────┘ └──────────────┘
```

## Initialization Order

### Critical Initialization Sequence

1. **HealthComponent** (Priority: 1)
   - No dependencies
   - Must exist before damage can be processed
   - Initializes poise and stagger systems

2. **VulnerabilityComponent** (Priority: 2)
   - No direct dependencies
   - Should exist before CombatComponent
   - Manages vulnerability states and i-frames

3. **CombatComponent** (Priority: 3)
   - Dependencies: HealthComponent, VulnerabilityComponent
   - Creates DamageCalculator in BeginPlay
   - Auto-creates VulnerabilityComponent if missing

4. **AnimationManagerComponent** (Priority: 4)
   - Dependencies: Character Mesh
   - Should be initialized after combat components
   - Binds to animation instance in BeginPlay

5. **DashComponent** (Priority: 5)
   - Dependencies: Character Movement Component
   - Can trigger vulnerability states
   - Should be initialized after combat system

6. **FocusModeComponent** (Priority: 6)
   - Dependencies: None direct
   - Interacts with IInteractable actors
   - Can be initialized at any time

7. **WallImpactComponent** (Priority: 7)
   - Dependencies: HealthComponent (for stagger)
   - Monitors knockback from damage
   - Should be initialized after combat system

8. **DebugRenderComponent** (Priority: 8)
   - Dependencies: None
   - Can be added dynamically
   - Auto-creates if requested via GetDebugRenderer()

## Component Communication Patterns

### Event-Driven Communication
Components communicate primarily through:
- **Delegates/Events**: For decoupled notifications
- **Interfaces**: For optional functionality (IStaggerable, IBlockable)
- **Direct Access**: Only when absolutely necessary

### Key Communication Flows

#### Damage Flow
```
Attack Input → CombatComponent → DamageCalculator → HealthComponent
                                      ↓
                              VulnerabilityComponent
```

#### Stagger Flow
```
Poise Damage → HealthComponent → OnStaggered Event
                                      ↓
                    [AnimationManager, CombatComponent, DashComponent]
```

#### Animation Flow
```
Combat Action → CombatComponent → AnimationManager → AnimInstance
                                      ↓
                              Animation Notifies → HitboxComponent
```

## Component Dependencies

### Hard Dependencies (Required)
- **CombatComponent** → HealthComponent, VulnerabilityComponent
- **DamageCalculator** → HealthComponent, CombatComponent
- **AnimationManager** → Character, AnimInstance
- **DashComponent** → CharacterMovementComponent

### Soft Dependencies (Optional)
- **CombatComponent** ↔ AnimationManager
- **FocusModeComponent** ↔ IInteractable actors
- **WallImpactComponent** ↔ HealthComponent
- **DebugRenderComponent** ↔ All components (for visualization)

## Best Practices

### Component Creation
1. Always check for existing components before creating
2. Use FindComponentByClass for runtime queries
3. Register dynamically created components

### Component Access
```cpp
// Preferred: Cache in BeginPlay
void UCombatComponent::BeginPlay()
{
    HealthComp = GetOwner()->FindComponentByClass<UHealthComponent>();
    ensure(HealthComp); // Soft assert in development
}

// Avoid: Repeated lookups
void UCombatComponent::ProcessDamage()
{
    // BAD: Looking up every time
    auto* Health = GetOwner()->FindComponentByClass<UHealthComponent>();
}
```

### Event Broadcasting
```cpp
// Standard pattern: Change State → Broadcast → Side Effects
void UHealthComponent::TakeDamage(float Damage)
{
    // 1. Update state
    CurrentHealth -= Damage;
    
    // 2. Broadcast event
    OnDamageTaken.Broadcast(Damage, Instigator);
    
    // 3. Handle side effects
    if (CurrentHealth <= 0)
    {
        HandleDeath();
    }
}
```

## Testing Checklist

### Component Initialization
- [ ] Components initialize in correct order
- [ ] Missing dependencies are handled gracefully
- [ ] Dynamic component creation works correctly

### Event Flow
- [ ] Events broadcast in correct order
- [ ] No circular dependencies in event handlers
- [ ] Events can be received by Blueprint

### Performance
- [ ] Components cache references in BeginPlay
- [ ] No unnecessary FindComponent calls per frame
- [ ] Debug rendering disabled in shipping builds

## Common Issues and Solutions

### Issue: Component not found at runtime
**Solution**: Ensure component is added in Blueprint or created in C++ constructor/BeginPlay

### Issue: Events not firing
**Solution**: Check delegate bindings and ensure components exist when binding

### Issue: Initialization order problems
**Solution**: Use BeginPlay for dependencies, Constructor for self-contained setup

### Issue: Circular dependencies
**Solution**: Use interfaces or events instead of direct component references

## Adding New Components

When adding a new component:
1. Define its dependencies clearly
2. Determine initialization priority
3. Use interfaces for optional interactions
4. Document communication patterns
5. Add to this architecture document

## Data Asset Dependencies

Components rely on these data assets:
- **CombatRulesDataAsset**: Combat calculations, multipliers, timings
- **AttackDataAsset**: Per-attack configurations
- **DebugCommandsDataAsset**: Debug command definitions

Ensure data assets are configured in:
- Blueprint component defaults
- Level blueprint
- Game instance