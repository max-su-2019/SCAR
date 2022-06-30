# Moveset(动作集)适配SCAR教程
本教程将会指引你如何对你的moveset(动作集，以下简称moveset)进行修改，以支持SCAR的自定义NPC连招AI功能。  
教程将会分成两个阶段进行解析：  
*  **攻击预备阶段** 
* **攻击连招阶段**
   
<br/> 

## 攻击预备阶段
---  
要为你的Moveset的第一下出手攻击添加SCAR的AI行动数据- *SCAR Action Data* (以下简称ActionData)，首先需要对角色的 **行为(behavior)** 进行修改：通过在角色行为的 **攻击预备状态机(AttackReadyStateMachine)** 下添加一个 **虚设动画(Dummy Animation)** ,以作为储存记录第一下出手攻击对应的ActionData的容器。
<br/>  
幸运的是，对于**人物角色(character)** 的攻击行为，SCAR自带的Nemesis行为补丁里已经为其添加了一个命名为`SCAR_1hmReadyDummy.hkx`的虚设动画文件；因此对于人物角色你无需自己修改行为或制作行为补丁(如果是生物moveset则需要)，只需将Moveset第一次出手攻击对应的ActionData通过 **注解（annotation）** 的方式加进`SCAR_1hmReadyDummy.hkx`动画文件并将此动画文件包含进你的Moveset中即可。  
<br/> 
以下是一条储存ActionData的注解的例子：

```
1.000000 SCAR_ActionData{"IdleAnimation":"ADXP_NPCPowerAttack", "MinDistance":0, "MaxDistance":139, "StartAngle":-60, "EndAngle":60, "Chance":30, "Type":"RPA"}
```
注解的内容可以分解为三个主要部分，如下图所示：   
![1](../images/SCAR%20Action%20Data.jpg) 
*  `weight`: 最左边的`1.000000`原本是作为注解触发的时间，这里用来表示的是 **权重(weight)**。权重用来决定ActionData的执行顺序：当一个动画文件里有多条ActionData时，会依据权重大小从高往低依次进行条件判定并选中首个符合条件的 ActionData 来执行对应攻击动作。  
     
* `prefix text`: 中间的`SCAR_ActionData`字段则是一个前缀标识字符，本身不包含任何有效数据，只是用来让SCAR从诸多注解中识别出带有 ActionData 的注解出来。  
   
* `json data`: 最后的部分是两个花括号包含着的长字段，这是一个标准的JSON格式字符串，里面储存的正是一个ActionData所应包含的几项主要数据：
    * `IdleAnimation` : 存储一条 ActionData 中所使用的 **闲置动画([Idle Animation](https://www.creationkit.com/index.php?title=Idle_Animations))** 的EditorID,当所有 ActionData 的条件都满足时便会播放执行此闲置动画。
    闲置动画需在esp中定义，*ADXP V1.4* 及以上版本已经包含了两个可用的闲置动画`ADXP_NPCPowerAttack` 和 `ADXP_NPCNormalAttack`，分别可用来执行NPC重击和轻击。闲置动画的执行还需满足闲置动画自身在esp中设置的条件。  
    *  `MaxDistance`: 表示这条 ActionData 的最大执行距离范围，仅当Moveset的持有者与其攻击目标之间的距离不大于这个数值时，此条ActionData才可以被执行。距离的值必须大于0。  
    注意这里的MaxDistance不需要考虑武器的距离，武器的距离会在实际计算最大距离时动态地加上。
    *  `MinDistance`: 表示这条ActionData的最小执行距离范围，仅当Moveset的持有者与其攻击目标之间的距离不小于这个数值时，此条ActionData才可以被执行。距离的值必须大于0。  
    最小距离可用于限制远距离的攻击只在与攻击目标距离达到一定大小时才执行。  
    *  `StartAngle` & `EndAngle` : 这两项数值共同用于表示ActionData执行所需的一个朝向角角度范围，仅当Moveset的持有者与其攻击目标之间的朝向角范围位于此角度范围之内时，ActionData才可以被执行。角度的有效取值大小为-180至180，朝向角范围的生成方式是从 `StartAngle` 开始、`EndAngle` 结束、以顺时针方向绘制的一个圆弧，范围的生成可参照[实例图片1](https://raw.githubusercontent.com/max-su-2019/SCAR/main/docs/images/Scar%20Angle%20Range%2001.JPG)和[实例图片2](https://raw.githubusercontent.com/max-su-2019/SCAR/main/docs/images/Scar%20Angle%20Range%2002.JPG)。
    * `Chance`: 表示执行这条ActionData的几率，几率的有效取值大小为0到100。
    *  `Type`: 表示ActionData要执行的动作的类型，动作类型会对攻击数值（轻重击数值）、武器距离等造成影响，目前可用的类型有以下几种：  
    *RA - RightAttack*  
    *RPA - RightPowerAttack*  
    *LA - LeftAttack*  
    *LPA - LeftPowerAttack*  
    *DA - DualAttack*  
    *DPA - DualPowerAttack*   
    *BA - BashAttack*  
    *BPA - BashPowerAttack*  
    *IDLE - Regular Idle*   
<br/> 

下面以ADXP V1.4默认双手斧动作（既SCAR默认动作包20006）Moveset中的 `SCAR_1hmReadyDummy.hkx` 动画里的SCAR注解作为例子，来说明SCAR如何读取其中的数据做AI判定：
```
1.000000 SCAR_ActionData{"IdleAnimation":"ADXP_NPCPowerAttack", "MinDistance":120, "MaxDistance":247, "StartAngle":-45, "EndAngle":45, "Chance":30, "Type":"RPA"}
0.500000 SCAR_ActionData{"IdleAnimation":"ADXP_NPCNormalAttack", "MinDistance":0, "MaxDistance":48, "StartAngle":-60, "EndAngle":60, "Chance":100, "Type":"RA"}
```
上面有两条 ActionData 数据注解，SCAR会首先根据权重来进行排序并依次逐个做判定，由于第一条 ActionData 的权重为1.000000、第二条权重为0.500000，所以SCAR会先对第一条ActionData 进行条件判定：  
* 当攻击目标的距离处于120到（247+武器距离）的范围内、攻击目标的角度范围处于-45到45度时，则NPC有30%的几率执行esp中名为 `ADXP_NPCPowerAttack` 的闲置动画，类型为 *RightPowerAttack* - 右手重击。在ADXP框架下，这意味着NPC会播放`mco_powerattack1.hkx`攻击动画。  

若第一条 ActionData 的不符合条件或闲置动画未能成功执行，则会继续往下执行权重较低的第二条 ActionData：  
* 当攻击目标的距离处于0到（48+武器距离）的范围内、攻击目标的角度范围位于-60到60度时，则NPC有100%的几率执行esp中名为 `ADXP_NPCNormalAttack` 的闲置动画，类型为 *RightAttack* - 右手攻击。在ADXP框架下，这意味着NPC会播放`mco_attack1.hkx`攻击动画。  

---    
<br/> <br/> 

## 攻击连招阶段
---  
要为攻击连招阶段添加SCAR支持，首先需要你在攻击动画所属的 **行为图(Behavior Graph)** 中添加一个名为 `SCAR_ComboStart` 的 **动作事件(animation event)** ，用来作为执行下一个连招攻击的AI判定窗口。  
幸运的是，SCAR和ADXP V1.4的行为补丁中已经添加了此动作事件，你无需自己修改行为或制作行为补丁(如果是生物moveset则需要)。  

下一步需要找到moveset里所有会继续衔接连招到下一个攻击的攻击动画文件，并为其添加表示可衔接到的下一个连招攻击动作的 ActionData 注解，方式和 `SCAR_1hmReadyDummy.hkx` 中的相同。  
除此之外，还需额外在你认为可开始执行衔接到下一个攻击动作的窗口时间里，加上 `SCAR_ComboStart` 这个动作事件。 对于ADXP而言，`SCAR_ComboStart` 的时间应位于 MCO_WinOpen 和 MCO_WinClose 之间、 或位于 MCO_PowerWinOpen 和 MCO_PowerWinClose 之间。  

以下是ADXP V1.4默认双手斧动作（既SCAR默认动作包20006）Moveset中的 `mco_attack1.hkx` 动画内的SCAR相关注解:  
```
1.550000 SCAR_ComboStart
1.000000 SCAR_ActionData{"IdleAnimation":"ADXP_NPCPowerAttack", "MinDistance":0, "MaxDistance":29, "StartAngle":-60, "EndAngle":60, "Chance":30, "Type":"RPA"}
0.500000 SCAR_ActionData{"IdleAnimation":"ADXP_NPCNormalAttack", "MinDistance":0, "MaxDistance":193, "StartAngle":-60, "EndAngle":60, "Chance":100, "Type":"RA"}
```
上述注解代表：当  `mco_attack1.hkx` 动画执行到1.550000秒时，会对注解中的两条 ActionData 进行判定和执行。

---

