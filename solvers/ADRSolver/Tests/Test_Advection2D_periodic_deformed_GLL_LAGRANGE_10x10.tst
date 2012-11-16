<?xml version="1.0" encoding="utf-8"?>
<test>
    <description>2D unsteady FRDG advection GLL_LAGRANGE, P=6, periodic bcs, deformed elements</description>
    <executable>ADRSolver</executable>
    <parameters>Test_Advection2D_periodic_deformed_GLL_LAGRANGE_10x10.xml</parameters>
    <files>
        <file description="Session File">Test_Advection2D_periodic_deformed_GLL_LAGRANGE_10x10.xml</file>
    </files>
    <metrics>
        <metric type="L2" id="1">
            <value variable="u" tolerance="1e-12"> 0.000174876 </value>
        </metric>
        <metric type="Linf" id="2">
            <value variable="u" tolerance="1e-12"> 0.00369469 </value>
        </metric>
    </metrics>
</test>