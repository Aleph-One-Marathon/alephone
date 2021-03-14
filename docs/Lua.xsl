<?xml version="1.0" encoding="UTF-8"?>

<xsl:transform version="1.0" xmlns:xsl="http://www.w3.org/1999/XSL/Transform">
<xsl:output method="html" encoding="utf-8" omit-xml-declaration="yes" doctype-public="-//W3C//DTD HTML 4.01//EN" doctype-system="http://www.w3.org/TR/html4/strict.dtd" />
<xsl:template match="document">
  <html>
    <head>
      <title><xsl:value-of select="@title"/></title>
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
      
      h3 {
	  color: #003300;
	  margin-bottom: 4px;
	  padding-bottom: 1px;
	  border-bottom: thin solid;
      }

      h4 {
	  margin: 0px;
	  color: #003300;
      }

      div.tables p {
	  margin: 4px;
      }

	  div.tables > p {
	  margin-left: 0;
	  }

      p.note {
	  color: #666666;
	  font-style: italic;
      }

      ul {
	  list-style-type: none;
      }

      ul.args li {
	  margin: 10 px;
      }

      span.access {
	  color: #666666;
	  font-style: italic;
      }

      span.version {
	  color: #000066;
	  font-style: normal;
	  font-size: 8pt;
	  color: #FFFFFF;
	  background-color: #006600;
	  padding-left: 1px;
	  padding-right: 1px;
      }
      
      span.pre {
	  display: block;
	  margin: 4px 0;
	  white-space: pre-wrap;
      }

      div.end-mnemonics {
	  clear: both;
      }

      div.mnemonics-column {
	  float: left;
	  #width: 250px;
	  width: 33%;
      }

      ul.mnemonics li {
	  color: #006600;
      }

--&gt;
      </style>
    </head>
    <body>
      <h1>Table of Contents</h1>
      <ol>
      <xsl:for-each select="section|triggers|tables|types">
	<xsl:call-template name="toc"/>
      </xsl:for-each>
      </ol>
      <xsl:apply-templates select="section|triggers|tables|types"/>
    </body>
  </html>
</xsl:template>

<xsl:template name="toc">
  <li>
    <a><xsl:attribute name="href">#<xsl:value-of select="@id"/></xsl:attribute>
    <xsl:choose>
      <xsl:when test="self::triggers">Triggers</xsl:when>
      <xsl:when test="self::tables">Tables</xsl:when>
      <xsl:when test="self::types">Types and Mnemonics</xsl:when>
      <xsl:otherwise>
	<xsl:value-of select="@name"/>
      </xsl:otherwise>
    </xsl:choose></a>
    <xsl:choose>
      <xsl:when test="section and count(ancestor::*) = 1">
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
      <xsl:when test="self::types">
	<ol>
	  <xsl:for-each select="../tables/enum-accessor">
	    <xsl:sort select="@nice-name"/>
	    <li><a><xsl:attribute name="href">#<xsl:value-of select="@name"/></xsl:attribute><xsl:value-of select="@nice-name"/></a></li>
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
    <dt>Triggers</dt>
    <dd>
  <dl>
    <xsl:apply-templates select="function"/>
  </dl>
    </dd>
    </dl>
  </div>
</xsl:template>

<xsl:template match="tables">
  <h1><a><xsl:attribute name="name"><xsl:value-of select="@id"/></xsl:attribute>Tables</a></h1>
  <div><xsl:attribute name="class">tables</xsl:attribute>
  <xsl:for-each select="description"><p><xsl:copy-of select="node()"/></p></xsl:for-each>
  <xsl:apply-templates select="accessor">
    <xsl:sort select="@name"/>
  </xsl:apply-templates>
  </div>
</xsl:template>

<xsl:template match="types">
  <h1><a><xsl:attribute name="name"><xsl:value-of select="@id"/></xsl:attribute></a>Types and Mnemonics</h1>
  <div class="tables">
    <xsl:for-each select="description"><p><xsl:copy-of select="node()"/></p></xsl:for-each>
    <xsl:apply-templates select="../tables/enum-accessor">
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
  <h3><a><xsl:attribute name="name"><xsl:value-of select="@name"/></xsl:attribute></a><xsl:value-of select="@nice-name"/>
    <xsl:choose>
      <xsl:when test="@version">
	<xsl:text> </xsl:text>
	<span class="version"><xsl:value-of select="@version"/></span>
      </xsl:when>
    </xsl:choose> 
  </h3>
  <dl>
    <dt># <xsl:value-of select="@name"/></dt>
    <dt><xsl:value-of select="@name"/>()</dt>
    <xsl:apply-templates select="call"/>
    <xsl:choose>
      <xsl:when test="count(id(@contains)/*) - count(id(@contains)/mnemonics) > 0">
	<dt><xsl:value-of select="@name"/>[<xsl:value-of select="@contains"/>]</dt>
	<dd>
	  <dl>
	    <xsl:apply-templates select="id(@contains)"/>
	  </dl>
	</dd>
      </xsl:when>
    </xsl:choose>
  </dl>
  <xsl:choose>
      <xsl:when test="id(@contains)/mnemonics">
	<h4>Mnemonics</h4>
	<xsl:variable name="total" select="count(id(@contains)/mnemonics/mnemonic)"/>
	  <xsl:choose>
	    <xsl:when test="$total > 8">
	      <xsl:variable name="third" select="ceiling($total div 3)"/>
		<div class="mnemonics-column"><ul class="mnemonics">
		  <xsl:for-each select="id(@contains)/mnemonics/mnemonic[position() &lt;= $third]">
		    <xsl:call-template name="mnemonic"/>
		  </xsl:for-each>
		</ul></div>
		<div class="mnemonics-column"><ul class="mnemonics">
		  <xsl:for-each select="id(@contains)/mnemonics/mnemonic[position() &gt; $third and position() &lt;= $third * 2]">
		    <xsl:call-template name="mnemonic"/>
		  </xsl:for-each>
		</ul></div>
		<div class="mnemonics-column"><ul class="mnemonics">
		  <xsl:for-each select="id(@contains)/mnemonics/mnemonic[position() &gt; $third * 2]">
		    <xsl:call-template name="mnemonic"/>
		  </xsl:for-each>
		</ul></div>
		<div class="end-mnemonics"/>
	    </xsl:when>
	    <xsl:otherwise>
	      <ul class="mnemonics">
		<xsl:for-each select="id(@contains)/mnemonics/mnemonic">
		  <xsl:call-template name="mnemonic"/>
		</xsl:for-each>
	      </ul>
	    </xsl:otherwise>
	  </xsl:choose>
      </xsl:when>
    </xsl:choose>
</xsl:template>

<xsl:template name="mnemonic">
  <li>"<xsl:value-of select="@name"/>"</li>
</xsl:template>

<xsl:template match="accessor">
  <h3><a><xsl:attribute name="name"><xsl:value-of select="@name"/></xsl:attribute></a><xsl:value-of select="@name"/><xsl:choose>
      <xsl:when test="@version">
	<xsl:text> </xsl:text>
	<span class="version"><xsl:value-of select="@version"/></span>
      </xsl:when>
    </xsl:choose></h3>
  <xsl:copy-of select="description/node()"/>
  <dl>
    <xsl:apply-templates select="length|call|function|variable"/>
    <xsl:choose>
      <xsl:when test="index">
	<dt><xsl:value-of select="@name"/>[index]</dt>
	<dd>
	  <xsl:choose>
	    <xsl:when test="id(@contains)/*">
	      <dl>
		<xsl:apply-templates select="id(@contains)"/>
	      </dl>
	    </xsl:when>
	  </xsl:choose>
	</dd>
      </xsl:when>
      <xsl:otherwise>
	<dt><xsl:value-of select="@name"/></dt>
	<dd>
	  <xsl:choose>
	    <xsl:when test="id(@contains)/description">
	      <p class="description"><xsl:copy-of select="id(@contains)/description/node()"/></p>
	    </xsl:when>
	  </xsl:choose>
	  <xsl:apply-templates select="id(@contains)/note"/>
	  <dl>
	    <xsl:apply-templates select="id(@contains)/function|id(@contains)/function-variable|id(@contains)/subtable|id(@contains)/subtable-accessor|id(@contains)/variable"/>
	  </dl>
	</dd>
      </xsl:otherwise>
    </xsl:choose>
  </dl>
</xsl:template>

<xsl:template match="subtable">
  <dt>.<xsl:value-of select="@name"/>
    <xsl:choose>
      <xsl:when test="@version">
	<xsl:text> </xsl:text>
	<span class="version"><xsl:value-of select="@version"/></span>
      </xsl:when>
    </xsl:choose> 
  <xsl:for-each select="alias"><br/>.<xsl:value-of select="."/></xsl:for-each>
  </dt>
  <dd>
    <xsl:apply-templates select="note"/>
    <xsl:choose>
      <xsl:when test="description">
	<p class="description"><xsl:copy-of select="description/node()"/></p>
      </xsl:when>
    </xsl:choose>
    <dl><xsl:apply-templates select="function|function-variable|variable|subtable|subtable-accessor"><xsl:sort select="@name"/></xsl:apply-templates>
  </dl></dd>
</xsl:template>

<xsl:template match="function">
  <dt>
    <xsl:choose>
      <xsl:when test="(parent::table and ../@singleton = 'false') or parent::subtable-accessor or parent::subtable">:</xsl:when>
      <xsl:when test="parent::table or parent::subtable">.</xsl:when>
      <xsl:when test="parent::triggers">.</xsl:when>
      <xsl:otherwise><xsl:value-of select="../@name"/>.</xsl:otherwise>
    </xsl:choose>
    <xsl:value-of select="@name"/>
    <xsl:choose>
      <xsl:when test="@named_parameters = 'true'">{</xsl:when>
      <xsl:otherwise>(</xsl:otherwise>
    </xsl:choose>
    <xsl:for-each select="argument">
      <xsl:choose><xsl:when test="@required = 'false'"> [</xsl:when></xsl:choose>
      <xsl:choose>
	<xsl:when test="position() != 1">, </xsl:when>
      </xsl:choose>
      <xsl:value-of select="@name"/><xsl:if test="../@named_parameters='true'">=</xsl:if><xsl:choose><xsl:when test="@required = 'false'">]</xsl:when></xsl:choose>
	  <xsl:choose>
		<xsl:when test="@version">
		  <xsl:text> </xsl:text><span class="version"><xsl:value-of select="@version"/></span>
		</xsl:when>
	  </xsl:choose>
    </xsl:for-each>
    <xsl:choose>
      <xsl:when test="@named_parameters = 'true'">}</xsl:when>
      <xsl:otherwise>)</xsl:otherwise>
    </xsl:choose>
    <xsl:choose>
      <xsl:when test="@version">
	<xsl:text> </xsl:text>
	<span class="version"><xsl:value-of select="@version"/></span>
      </xsl:when>
    </xsl:choose>
  </dt>
  <dd><p class="description"><xsl:copy-of select="description/node()"/></p>
  <xsl:choose>
    <xsl:when test="argument/description">
      <ul class="args"><xsl:for-each select="argument/description"><li><xsl:value-of select="../@name"/>: <xsl:value-of select="."/></li></xsl:for-each></ul>
    </xsl:when>
  </xsl:choose>
  <xsl:apply-templates select="note"/>
  </dd>
</xsl:template>

<xsl:template match="variable">
  <dt>
	<xsl:choose>
	  <xsl:when test="parent::accessor"><xsl:value-of select="../@name"/>.</xsl:when>
	  <xsl:otherwise>.</xsl:otherwise>
	</xsl:choose>
    <xsl:value-of select="@name"/>
    <xsl:choose>
      <xsl:when test="@access = 'local-player read-only'">
	<span class="access"> (read-only) (local player)</span>
      </xsl:when>
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
      <xsl:if test="substring(., 1, 1) != '['">.</xsl:if><xsl:value-of select="."/>
      <xsl:choose>
	<xsl:when test="../@access = 'local-player read-only'">
	  <span class="access"> (read-only) (local player)</span>
	</xsl:when>
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
    <xsl:choose>
      <xsl:when test="@version">
	<xsl:text> </xsl:text>
	<span class="version"><xsl:value-of select="@version"/></span>
      </xsl:when>
    </xsl:choose>
  </dt>
  <dd><p class="description"><xsl:value-of select="description/node()"/></p>
  <xsl:apply-templates select="note"/>
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
    .<xsl:value-of select="@name"/>[<xsl:value-of select="@index"/>]<xsl:choose>
	<xsl:when test="@version">
	  <xsl:text> </xsl:text>
	  <span class="version"><xsl:value-of select="@version"/></span>
	</xsl:when>
  </xsl:choose><xsl:choose>
      <xsl:when test="@access = 'local-player'">
      <span class="access"> (local player)</span>
      </xsl:when>
    </xsl:choose></dt>
  <dd>
    <xsl:choose>
      <xsl:when test="description">
	<p class="description"><xsl:copy-of select="description/node()"/></p>
      </xsl:when>
    </xsl:choose>
    <xsl:choose>
      <xsl:when test="function|subtable|subtable-accessor|variable">
	<dl><xsl:apply-templates select="function|subtable|subtable-accessor|variable"><xsl:sort select="@name"/></xsl:apply-templates></dl>
      </xsl:when>
    </xsl:choose>
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

<xsl:template match="note">
  <p class="note">
  <xsl:value-of select="."/>
  <xsl:text> </xsl:text>
  <xsl:choose>
    <xsl:when test="@version">
      <span class="version"><xsl:value-of select="@version"/></span>
    </xsl:when>
  </xsl:choose>
  </p>
</xsl:template>

</xsl:transform>
