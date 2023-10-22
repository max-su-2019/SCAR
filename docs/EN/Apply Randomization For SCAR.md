# Apply Randomization For SCAR Moveset

This tutorial would instruct you about how to properly create random moveset that could works with SCAR.  
Please read about the [basic tutorial](/How%20To%20Patch%20Moveset%20For%20SCAR.md) before come into this one!

## Issue Descriptions

When designing moveset for some special NPCs, such as boss enemy, you may meet the requirements to allow them perform two or more different set of combos attack animations randomly.  
However, due to the ways how **Dynamic Animation Replacer** handle animations randomization, you cannot do that properly without mess up the SCAR AI data, since the SCAR AI data of the next combos attack animation is stored inside the previous one. But now, with the new **Animation variants** feature from **Open Animation Replacer** and the API it provided, following with the new update of SCAR plugin, we finally get the workaround to implement it correctly.

## Requirements

- Open Animation Replacer v2.02 and newer
- SCAR v1.11 and newer
- Behavior Data Injector

## Core Stuffs

1. A graph variable int named `SCAR_AttackVariants` was added via **Behavior Data Injector**, this value storing and represents the serial number of the attack animations set that was picked at random.
2. With the help of **Animation variants** feature from **Open Animation Replacer**, and the API it provied, we can randomize animation files within a varaint folder and get the result of the file picked at random.
3. With the v1.1.1 update of SCAR, you can assign suffix text with fomart `_scarVar$[number]` within the name of a variants animation file that contains SCAR annoatations, the suffix could then set up the value of the `SCAR_AttackVariants` to the serial value it defined.  
   For instance, a variant animation file named "SCAR_1hm_scarVar$2.hkx" would set the value of `SCAR_AttackVariants` to 2 when it activated, while a variant animation file named "SCAR_1hm_scarVar$3.hkx" would set the value of `SCAR_AttackVariants` to 3, etc.

## Practical Example

Here I would show you a practical example about how to create SCAR random moveset.  
First of all, please download the files of the example moveset from this [link](https://github.com/max-su-2019/Random-SCAR-Movest-Example). After extracte the file, open up the "Separated Moveset" folder, inside you could see two folders, each of them storing a compelety separated SCAR moveset folder, Which is "Bandit Sword Moveset 0" and "Bandit Sword Moveset 1", separately.

```
Separated Moveset
├─Bandit Sword Moveset 0
│      mco_attack1.hkx
│      mco_attack2.HKX
│      mco_attack3.HKX
│      mco_powerattack1.hkx
│      mco_powerattack2.HKX
│      mco_powerattack3.hkx
│      mco_powerattack4.HKX
│      SCAR_1hmReadyDummy.hkx
│      SCAR_BlockIdleDummy.hkx
│
└─Bandit Sword Moveset 1
        mco_attack1.hkx
        mco_attack2.hkx
        mco_attack3.hkx
        mco_powerattack1.hkx
        SCAR_1hmReadyDummy.hkx
        SCAR_BlockIdleDummy.hkx
```

Next, I would show you how to merge this two separated moveset together as a Random SCAR Moveset, after then NPC could randomly pick up one of them to perform for an attack. To explaint how it works, Let 's take a look of the files inside the final completion "Merged Moveset" folder, which already had both of the "Bandit Sword Moveset 0" and "Bandit Sword Moveset 1" animations included and built up for randomization.

```
Merged Moveset
└─OpenAnimationReplacer
    └─Random Bandit Sword Moveset
        │  config.json
        │
        ├─SCARAttackVariants0
        │      config.json
        │      mco_attack1.hkx
        │      mco_attack2.HKX
        │      mco_attack3.HKX
        │      mco_powerattack1.hkx
        │      mco_powerattack2.HKX
        │      mco_powerattack3.hkx
        │      mco_powerattack4.HKX
        │
        ├─SCARAttackVariants1
        │      config.json
        │      mco_attack1.hkx
        │      mco_attack2.hkx
        │      mco_attack3.hkx
        │      mco_powerattack1.hkx
        │
        └─SCARDummyClips
            │  config.json
            │
            ├─_variants_SCAR_1hmReadyDummy
            │      SCAR_1hm_scarVar$0.hkx
            │      SCAR_1hm_scarVar$1.hkx
            │
            └─_variants_SCAR_BlockIdleDummy
                    SCAR_Block_scarVar$0.hkx
                    SCAR_Block_scarVar$1.hkx
```

Inside the folder, A complete OAR animation mod named "Random Bandit Sword Moveset" was contained, following with three sub mod folders. The first sub mod folder that need to be created is the "SCARDummyClips" folder, it contains the OAR variant renamed copies of the original "SCAR_1hmReadyDummy.hkx" and "SCAR_BlockIdleDummy.hkx" animation files from the "Bandit Sword Moveset 0" and "Bandit Sword Moveset 1", Those animation files were renamed as

For "Bandit Sword Moveset 0":

- "SCAR_1hm_scarVar\$0.hkx"
- "SCAR_Block_scarVar\$0.hkx"

For "Bandit Sword Moveset 1":

- "SCAR_1hm_scarVar\$1.hkx"
- "SCAR_Block_scarVar\$1.hkx"

and you can easily find out the names of these variant animation files are all attached with the suffix `_scarVar$[Number]`, therefore once these dummy animation were activated, the value of `SCAR_AttackVariants` variable would be set as the suffix serial number of their file names.

Then, we can create the sub mod folder "SCARAttackVariants0" and "SCARAttackVariants1", we copy and paste all of those MCO attack animations from "Bandit Sword Moveset 0" into "SCARAttackVariants0", while the files from "Bandit Sword Moveset 1" should put into "SCARAttackVariants1".

After that, we come to the stage of assigining OAR condtions for the moveset. First we simply copy the basic conditions function of the origianl sperate moveset and paste them into the three sub mods "SCARDummyClips", "SCARAttackVariants0", "SCARAttackVariants1" within the merged moveset ,separately. You can do that by editing the OAR config josn file or using the in-game editor menu.

Next, For the "SCARAttackVariants0" sub mod, which contains the orignal "Bandit Sword Moveset 0" moveset, we need to assign one extra condition function for it:

```json
{
  "condition": "CompareValues",
  "requiredVersion": "1.0.0.0",
  "Value A": {
    "graphVariable": "SCAR_AttackVariants",
    "graphVariableType": "Int"
  },
  "Comparison": "==",
  "Value B": {
    "value": 0.0
  }
}
```

The condtion function above check if the current value of variable `SCAR_AttackVariants` is equal to 0, if that 's true which means the "SCAR_1hm_scarVar$0.hkx" or "SCAR_Block_scarVar$0.hkx" is activated, then we should make moveset "SCARAttackVariants0" enabled which also means the npc would perform "Bandit Sword Moveset 0" in his coming attack.

For the same reason, we should assign this condtion function for "SCARAttackVariants1" sub mod:

```json
{
  "condition": "CompareValues",
  "requiredVersion": "1.0.0.0",
  "Value A": {
    "graphVariable": "SCAR_AttackVariants",
    "graphVariableType": "Int"
  },
  "Comparison": "==",
  "Value B": {
    "value": 1.0
  }
}
```

So above is the complete workflow of creating the practical example moveset. And if you wanna to add one more moveset into the merged "Random Bandit Sword Moveset", let's assume this new added moveset is called "Bandit Sword Moveset 2" with files below:

```
└─Bandit Sword Moveset 2
    mco_attack1.hkx
    mco_attack2.hkx
    mco_attack3.hkx
    mco_powerattack1.hkx
    mco_powerattack2.HKX
    mco_powerattack3.hkx
    mco_powerattack4.hkx
    mco_powerattack5.hkx
    mco_powerattack6.hkx
    SCAR_1hmReadyDummy.hkx
    SCAR_BlockIdleDummy.hkx
```

To merge this moveset into "Random Bandit Sword Moveset", firstly you should copy dummy animation files "SCAR_1hmReadyDummy.hkx" and "SCAR_BlockIdleDummy.hkx" into the "SCARDummyClips" folder, and rename them with the suffix `_scarVar$2`, such as "SCAR_1hm_scarVar\$2.hkx" and "SCAR_Block_scarVar\$2.hkx".

Then simply create a sub mod folder named "SCARAttackVariants2", and copy all of those MCO attack animations inside the folder.

After that, assigin this OAR conditon for "SCARAttackVariants2" sub mod:

```json
{
  "condition": "CompareValues",
  "requiredVersion": "1.0.0.0",
  "Value A": {
    "graphVariable": "SCAR_AttackVariants",
    "graphVariableType": "Int"
  },
  "Comparison": "==",
  "Value B": {
    "value": 2.0
  }
}
```

That 's all! Now the "Bandit Sword Moveset 2" above should now able to merge in to random SCAR moveset and can be pick up to pefrom during attack. The file struct of the "Random Bandit Sword Moveset" after merged should be as below:

```
Random Bandit Sword Moveset
│  config.json
│
├─SCARAttackVariants0
│      config.json
│      mco_attack1.hkx
│      mco_attack2.HKX
│      mco_attack3.HKX
│      mco_powerattack1.hkx
│      mco_powerattack2.HKX
│      mco_powerattack3.hkx
│      mco_powerattack4.HKX
│
├─SCARAttackVariants1
│      config.json
│      mco_attack1.hkx
│      mco_attack2.hkx
│      mco_attack3.hkx
│      mco_powerattack1.hkx
│
├─SCARAttackVariants2
│      config.json
│      mco_attack1.hkx
│      mco_attack2.hkx
│      mco_attack3.hkx
│      mco_powerattack1.hkx
│      mco_powerattack2.HKX
│      mco_powerattack3.hkx
│      mco_powerattack4.hkx
│      mco_powerattack5.hkx
│      mco_powerattack6.hkx
│
└─SCARDummyClips
    │  config.json
    │
    ├─_variants_SCAR_1hmReadyDummy
    │      SCAR_1hm_scarVar$0.hkx
    │      SCAR_1hm_scarVar$1.hkx
    │      SCAR_1hm_scarVar$2.hkx
    │
    └─_variants_SCAR_BlockIdleDummy
            SCAR_Block_scarVar$0.hkx
            SCAR_Block_scarVar$1.hkx
            SCAR_Block_scarVar$2.hkx
```
