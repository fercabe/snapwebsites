<?xml version="1.0"?>
<!--
Snap Websites Server == e-Payment global settings parser
Copyright (c) 2014-2018  Made to Order Software Corp.  All Rights Reserved

This program is free software; you can redistribute it and/or modify
it under the terms of the GNU General Public License as published by
the Free Software Foundation; either version 2 of the License, or
(at your option) any later version.

This program is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
GNU General Public License for more details.

You should have received a copy of the GNU General Public License
along with this program; if not, write to the Free Software
Foundation, Inc., 51 Franklin St, Fifth Floor, Boston, MA  02110-1301  USA
-->
<xsl:stylesheet version="2.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform"
															xmlns:xs="http://www.w3.org/2001/XMLSchema"
															xmlns:fn="http://www.w3.org/2005/xpath-functions"
															xmlns:snap="snap:snap">

	<!-- some special variables to define the theme -->
	<xsl:variable name="layout-area">epayment-settings-parser</xsl:variable>
	<xsl:variable name="layout-modified">2016-04-15 15:56:45</xsl:variable>
	<xsl:variable name="layout-editor">epayment-settings-page</xsl:variable>

	<xsl:template match="snap">
		<output><!-- lang="{$lang}"-->
			<div class="editor-form" form_name="epayment">
				<xsl:attribute name="session"><xsl:value-of select="page/body/editor/session/div/div/node()"/></xsl:attribute>
				<xsl:attribute name="timeout"><xsl:value-of select="page/body/editor/timeout/div/div/node()"/></xsl:attribute>

				<h2>e-Payment Settings</h2>
				<div>

					<fieldset>
						<legend>Store Information</legend>

						<div class="editor-block">
							<label for="store_name" class="settings-title">Store Name</label>
							<xsl:copy-of select="page/body/epayment/store_name/node()"/>
						</div>

						<div class="editor-block">
							<label for="store_address1" class="settings-title">Address</label>
							<xsl:copy-of select="page/body/epayment/store_address1/node()"/>
							<xsl:copy-of select="page/body/epayment/store_address2/node()"/>
						</div>

						<div class="editor-block">
							<label for="store_city" class="settings-title">City</label>
							<xsl:copy-of select="page/body/epayment/store_city/node()"/>
						</div>

						<div class="editor-block">
							<label for="store_province" class="settings-title">Province or State</label>
							<xsl:copy-of select="page/body/epayment/store_province/node()"/>
						</div>

						<div class="editor-block">
							<label for="store_postal_code" class="settings-title">Postal Code</label>
							<xsl:copy-of select="page/body/epayment/store_postal_code/node()"/>
						</div>

						<div class="editor-block">
							<label for="store_country" class="settings-title">Country</label>
							<xsl:copy-of select="page/body/epayment/store_country/node()"/>
						</div>

					</fieldset>

				</div>
			</div>

		</output>
	</xsl:template>
</xsl:stylesheet>
<!-- vim: ts=2 sw=2
-->
