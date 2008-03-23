<?xml version="1.0" encoding="UTF-8"?>

<xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html" encoding="utf-8" omit-xml-declaration="yes"/>
<xsl:template match="document">
  <html>
    <head>
      <title>Aleph One Lua Scripters&apos; Guide</title>
      <style type="text/css">
      &lt;!-- 
      body {
	  font-family: Verdana, Helvetica, sans-serif;
	  font-size: 9pt;
	  width: 800px;
      }
      a:link {
	  color: #006600;
      }

      a:visited {
	  color: #669966;
      }
      
      a:hover {
	  color: #009900;
      }

      h1 {
	  background-color: #EEEEEE;
	  border: thin solid #999999;
	  padding: 4px;
      }
      h2 {
	  color: #006600;
      }
      dt {
	  color: #006600;
	  margin-top: 4px;
	  margin-bottom: 4px;
      }
      
      div.triggers dt {
	  border-bottom: thin solid;
	  border-color: #006600;
	  padding-bottom: 1px;
      }

      h3 {
	  color: #003300;
	  margin-bottom: 4px;
	  padding-bottom: 1px;
	  border-bottom: thin solid;
      }

      div.tables p {
	  margin: 4px;
      }

      p.note {
	  color: #666666;
	  font-style: italic;
      }

      ul.args {
	  list-style-type: none;
      }

      ul.args li {
	  margin: 10 px;
      }

      span.access {
	  color: #666666;
	  font-style: italic;
      }

--&gt;
      </style>
    </head>
    <body>
      <h1>Table of Contents</h1>
      <ol>
      <xsl:for-each select="section|triggers|tables">
	<xsl:call-template name="toc"/>
      </xsl:for-each>
      </ol>
      <xsl:apply-templates select="section|triggers|tables"/>
    </body>
  </html>
</xsl:template>

<xsl:template name="toc">
  <li>
    <a><xsl:attribute name="href">#<xsl:value-of select="@id"/></xsl:attribute>
    <xsl:choose>
      <xsl:when test="self::triggers">Triggers</xsl:when>
      <xsl:when test="self::tables">Tables</xsl:when>
      <xsl:otherwise>
	<xsl:value-of select="@name"/>
      </xsl:otherwise>
    </xsl:choose></a>
    <xsl:choose>
      <xsl:when test="self::section and count(ancestor::*) = 1">
	<ol>
	  <xsl:for-each select="section"><xsl:call-template name="toc"/></xsl:for-each>
	</ol>
      </xsl:when>
      <xsl:when test="self::tables">
	<ol>
	  <xsl:for-each select="accessor">
	    <xsl:sort select="@name"/>
	    <li><a><xsl:attribute name="href">#<xsl:value-of select="@name"/></xsl:attribute><xsl:value-of select="@name"/></a></li>
	  </xsl:for-each>
	</ol>
      </xsl:when>
    </xsl:choose>
  </li>
</xsl:template>

<xsl:template match="triggers">
  <h1><a><xsl:attribute name="name"><xsl:value-of select="@id"/></xsl:attribute></a>Triggers</h1>
  <div><xsl:attribute name="class">triggers</xsl:attribute>
  <p><xsl:copy-of select="description/node()"/></p>
  <dl>
    <xsl:apply-templates select="function"/>
  </dl>
  </div>
</xsl:template>

<xsl:template match="tables">
  <h1><a><xsl:attribute name="name"><xsl:value-of select="@id"/></xsl:attribute>Tables</a></h1>
  <div><xsl:attribute name="class">tables</xsl:attribute>
  <xsl:for-each select="description"><p><xsl:copy-of select="node()"/></p></xsl:for-each>
  <xsl:apply-templates select="accessor|enum-accessor">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>
  </div>
</xsl:template>

<xsl:template match="section">
  <xsl:choose>
    <xsl:when test="count(ancestor::*) = 1">
      <h1><a><xsl:attribute name="name"><xsl:value-of select="@id"/></xsl:attribute></a><xsl:value-of select="@name"/></h1>
    </xsl:when>
    <xsl:when test="count(ancestor::*) = 2">
      <h2><a><xsl:attribute name="name"><xsl:value-of select="@id"/></xsl:attribute></a><xsl:value-of select="@name"/></h2>
    </xsl:when>
  </xsl:choose>
  <xsl:copy-of select="description/node()"/>
  <xsl:apply-templates select="section"/>
</xsl:template>

<xsl:template match="enum-accessor">
  <h3><a><xsl:attribute name="name"><xsl:value-of select="@name"/></xsl:attribute></a><xsl:value-of select="@name"/></h3>
  <dl>
    <xsl:apply-templates select="length"/>
    <xsl:apply-templates select="call"/>
    <dt><xsl:value-of select="@name"/>[<xsl:value-of select="@contains"/>]</dt>
    <dd>
      <xsl:apply-templates select="id(@contains)"/>
    </dd>
  </dl>
</xsl:template>

<xsl:template match="accessor">
  <h3><a><xsl:attribute name="name"><xsl:value-of select="@name"/></xsl:attribute></a><xsl:value-of select="@name"/></h3>
  <dl>
    <xsl:apply-templates select="length|call|function"/>
    <xsl:choose>
      <xsl:when test="index">
	<dt><xsl:value-of select="@name"/>[index]</dt>
	<dd>
	  <dl>
	    <xsl:apply-templates select="id(@contains)"/>
	  </dl>
	</dd>
      </xsl:when>
      <xsl:otherwise>
	<dd>
	  <xsl:apply-templates select="id(@contains)"/>
	</dd>
      </xsl:otherwise>
    </xsl:choose>
  </dl>
</xsl:template>

<xsl:template match="subtable">
  <dt>.<xsl:value-of select="@name"/>
  <xsl:for-each select="alias"><br/>.<xsl:value-of select="."/></xsl:for-each>
  <xsl:for-each select="note"><p class="note"><xsl:value-of select="."/></p></xsl:for-each>
  </dt>
  <dd><dl><xsl:apply-templates select="function|function-variable|variable|subtable"><xsl:sort select="@name"/></xsl:apply-templates>
  </dl></dd>
</xsl:template>

<xsl:template match="function">
  <dt>
    <xsl:choose>
      <xsl:when test="(parent::table and ../@singleton = 'false') or parent::subtable-accessor">:</xsl:when>
      <xsl:when test="parent::table or parent::subtable">.</xsl:when>
      <xsl:when test="parent::triggers"></xsl:when>
      <xsl:otherwise><xsl:value-of select="../@name"/>.</xsl:otherwise>
    </xsl:choose>
    <xsl:value-of select="@name"/>(<xsl:for-each select="argument">
    <xsl:choose><xsl:when test="@required = 'false'"> [</xsl:when></xsl:choose>
    <xsl:choose>
      <xsl:when test="position() != 1">, </xsl:when>
    </xsl:choose>
    <xsl:value-of select="@name"/><xsl:choose><xsl:when test="@required = 'false'">]</xsl:when></xsl:choose>
    </xsl:for-each>)
  </dt>
  <dd><p class="description"><xsl:copy-of select="description/node()"/></p>
  <xsl:choose>
    <xsl:when test="argument/description">
      <ul class="args"><xsl:for-each select="argument/description"><li><xsl:value-of select="../@name"/>: <xsl:value-of select="."/></li></xsl:for-each></ul>
    </xsl:when>
  </xsl:choose>
  <xsl:for-each select="note">
    <p class="note"><xsl:value-of select="."/></p>
  </xsl:for-each>
  </dd>
</xsl:template>

<xsl:template match="variable">
  <dt>
    .<xsl:value-of select="@name"/>
    <xsl:choose>
      <xsl:when test="@access = 'read-only'">
	<span class="access"> (read-only)</span>
      </xsl:when>
      <xsl:when test="@access = 'local-player'">
	<span class="access"> (local player)</span>
      </xsl:when>
      <xsl:when test="@access = 'write-only'">
	<span class="access"> (write-only)</span>
      </xsl:when>
    </xsl:choose>
    <xsl:for-each select="alias">
      <br/>
      .<xsl:value-of select="."/>
      <xsl:choose>
	<xsl:when test="../@access = 'read-only'">
	  <span class="access"> (read-only)</span>
	</xsl:when>
	<xsl:when test="../@access = 'local-player'">
	  <span class="access"> (local player)</span>
	</xsl:when>
	<xsl:when test="../@access = 'write-only'">
	  <span class="access"> (write-only)</span>
	</xsl:when>
      </xsl:choose>
    </xsl:for-each>
  </dt>
  <dd><p class="description"><xsl:value-of select="description/node()"/></p>
  <xsl:for-each select="note">
    <p class="note"><xsl:value-of select="."/></p>
  </xsl:for-each>
  </dd>
</xsl:template>

<!-- these look like tables-->
<xsl:template match="function-variable">
  <dt>
    .<xsl:value-of select="@name"/>[<xsl:value-of select="argument/@name"/>]
  </dt>
  <dd><p class="description"><xsl:value-of select="description"/></p></dd>
</xsl:template>

<xsl:template match="subtable-accessor">
  <dt>
    .<xsl:value-of select="@name"/>[<xsl:value-of select="@index"/>]
  </dt>
  <dd>
    <xsl:choose>
      <xsl:when test="description">
	<p class="description"><xsl:copy-of select="description/node()"/></p>
      </xsl:when>
    </xsl:choose>
    <dl><xsl:apply-templates select="function|subtable|subtable-accessor|variable"><xsl:sort select="@name"/></xsl:apply-templates></dl>
  </dd>
</xsl:template>

<xsl:template match="length">
  <dt># <xsl:value-of select="../@name"/></dt>
  <dd><p class="description"><xsl:copy-of select="description/node()"/></p></dd>
</xsl:template>

<xsl:template match="call">
  <dt><xsl:value-of select="../@name"/>()</dt>
  <dd><p class="description"><xsl:copy-of select="description/node()"/></p></dd>
</xsl:template>

<xsl:template match="index">
  <dt>
    <xsl:choose>
      <xsl:when test="parent::subtable">
	.<xsl:value-of select="../@name"/>
      </xsl:when>
      <xsl:otherwise>
	<xsl:value-of select="../@name"/>
      </xsl:otherwise>
    </xsl:choose>[<xsl:choose>
      <xsl:when test="@type">
	<xsl:value-of select="@type"/>
      </xsl:when>
      <xsl:otherwise>index</xsl:otherwise>
    </xsl:choose>]
  </dt>
  <dd><p class="description"><xsl:copy-of select="description/node()"/></p></dd>
</xsl:template>

</xsl:transform>
