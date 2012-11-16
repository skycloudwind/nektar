<?xml version="1.0" encoding="utf-8"?>
<test>
    <description>3D unsteady DG advection, hexahedra, order 1, P=12,periodic bcs</description>
    <executable>ADRSolver</executable>
    <parameters>Test_Advection3D_m12_DG_hex_periodic.xml</parameters>
    <files>
        <file description="Session File">Test_Advection3D_m12_DG_hex_periodic.xml</file>
    </files>
    <metrics>
        <metric type="L2" id="1">
            <value variable="u" tolerance="1e-12"> 7.03905e-7 </value>
        </metric>
        <metric type="Linf" id="2">
            <value variable="u" tolerance="1e-03"> 1.10475e-5 </value>
        </metric>
    </metrics>
</test>