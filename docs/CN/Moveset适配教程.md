# Moveset(动作集)适配SCAR教程
本教程将会指引你如何对你的moveset(动作集，以下简称moveset)进行修改，以支持SCAR的自定义NPC连招AI功能。  
教程将会分成两个阶段进行解析：  
*  **攻击预备阶段** 
* **攻击连招阶段**

---  
<br/> 

## 攻击预备阶段
---  
要为你的Moveset的第一下出手攻击添加SCAR的AI行动数据- *SCAR Action Data* (以下简称ActionData)，首先需要对角色的 **行为(behavior)** 进行修改：通过在角色行为的 **攻击预备状态机(AttackReadyStateMachine)** 下添加一个 **虚设动画(Dummy Animation)** ,以作为储存记录第一下出手攻击对应的ActionData的容器。
<br/>  
幸运的是，对于**人物角色(character)** 的攻击行为，SCAR自带的Nemesis行为补丁里已经为其添加了一个命名为`SCAR_1hmReadyDummy.hkx`的虚设动画文件；因此对于人物角色你无需自己修改行为或做行为补丁(如果是生物moveset则需要)，只需将Moveset第一次出手攻击对应的ActionData通过 **注解（annotation）** 的方式加进`SCAR_1hmReadyDummy.hkx`动画文件并将此动画文件包含进你的moveset中即可。  
<br/> 
以下是一条储存ActionData的注解的例子：

```
1.000000 SCAR_ActionData{"IdleAnimation":"ADXP_NPCPowerAttack", "MinDistance":0, "MaxDistance":139, "StartAngle":-60, "EndAngle":60, "Chance":30, "Type":"RPA"}
```
注解的内容可以分解为三个主要部分，如下图所示：   
![1](../images/SCAR%20Action%20Data.jpg) 
*  `weight`: 最左边的`1.000000`原本是作为动画的时间，这里用来表示的是**权重(weight)**。权重用来决定ActionData的执行顺序：当一个动画文件里有多条ActionData时，会依据权重大小从高往低依次进行条件判定并选中首个符合条件的 ActionData 来执行对应攻击动作。  
     
* `prefix text`: 中间的`SCAR_ActionData`字段则是一个前缀标识字符，本身不包含任何有效数据，只是用来让SCAR从诸多注解中识别出带有ActionData的注解出来。  
   
* `json data`: 最后的部分是两个花括号包含着的长字段，这是一个标准的JSON格式字符串，里面储存的正是一个ActionData所应包含的几项主要数据：
    * `IdleAnimation` : 存储一条ActionData中所使用的闲置动画的EditorID,当所有ActionData的条件都满足时便会播放执行此闲置动画。  
    闲置动画需在esp中定义，*ADXP V1.4* 及以上版本已经包含了两个可用的闲置动画`ADXP_NPCPowerAttack` 和 `ADXP_NPCNormalAttack`，分别可用来执行NPC重击和轻击。
    *  `MaxDistance`: 表示这条ActionData的最大执行距离范围，仅当Moveset的持有者与其攻击目标之间的距离不大于这个数值时，此条ActionData才可以被执行。  
    注意这里的MaxDistance不需要考虑武器的距离，武器的距离会在实际计算最大距离时动态地加上。
    *  `MinDistance`: 表示这条ActionData的最小执行距离范围，仅当Moveset的持有者与其攻击目标之间的距离不小于这个数值时，此条ActionData才可以被执行。  
    这项数据可用于禁止远距离的攻击在与攻击目标距离较近时被使用。  
    *  `StartAngle` & `EndAngle` : 这两项数值共同用于表示ActionData执行所需的一个角度范围，仅当Moveset的持有者与其攻击目标之间的范围位于此角度范围之内时，ActionData才可以被执行。角度范围的生成方式是从 `StartAngle` 开始、`EndAngle` 结束、以顺时针方向绘制的一个圆弧，角度范围的例子可参照下面两张图所示：
    ![1](../images/Scar%20Angle%20Range%2001.JPG)
    ![2](../images/Scar%20Angle%20Range%2002.JPG)
    * `Chance`: 表示执行这条ActionData的几率，只有当随机取数的值大于此几率时，这条ActionData才会被执行。
    *  `Type`: 表示ActionData要执行的动作的类型，动作类型会对攻击数值（轻重击数值）、武器距离等造成影响，目前可用的 `Type` 有以下几种：  
    *RA - RightAttack*  
    *RPA - RightPowerAttack*  
    *LA - LeftAttack*  
    *LPA - LeftPowerAttack*  
    *DA - DualAttack*  
    *DPA - DualPowerAttack*   
    *BA - BashAttack*  
    *BPA - BashPowerAttack*  
    *IDLE - Regular Idle*  

---    
    


