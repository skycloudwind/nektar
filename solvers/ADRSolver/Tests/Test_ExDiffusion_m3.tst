<?xml version="1.0" encoding="utf-8"?>
<test>
    <description>2D unsteady DG explicit diffusion, order 4, P=3</description>
    <executable>ADRSolver</executable>
    <parameters>Test_ExDiffusion_m3.xml</parameters>
    <files>
        <file description="Session File">Test_ExDiffusion_m3.xml</file>
    </files>
    <metrics>
        <metric type="L2" id="1">
            <value variable="u" tolerance="1e-12">0.00712372</value>
        </metric>
        <metric type="Linf" id="2">
            <value variable="u" tolerance="1e-12">0.0195377</value>
        </metric>
    </metrics>
</test>



