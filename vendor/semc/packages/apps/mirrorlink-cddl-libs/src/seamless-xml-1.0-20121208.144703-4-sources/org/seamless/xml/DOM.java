 /*
 * Copyright (C) 2012 4th Line GmbH, Switzerland
 *
 * The contents of this file are subject to the terms of either the GNU
 * Lesser General Public License Version 2 or later ("LGPL") or the
 * Common Development and Distribution License Version 1 or later
 * ("CDDL") (collectively, the "License"). You may not use this file
 * except in compliance with the License. See LICENSE.txt for more
 * information.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 */
package org.seamless.xml;

import org.w3c.dom.Document;
import org.w3c.dom.Element;

import javax.xml.xpath.XPath;
import java.net.URI;

/**
 * Wraps a W3C document and provides an entry point for browsing the DOM (in subclasses).
 *
 * @author Christian Bauer
 */
public abstract class DOM {

    public static final URI XML_SCHEMA_NAMESPACE =
            URI.create("http://www.w3.org/2001/xml.xsd");

    public static final String CDATA_BEGIN = "<![CDATA[";
    public static final String CDATA_END = "]]>";

    private Document dom;

    public DOM(Document dom) {
        this.dom = dom;
    }

    public Document getW3CDocument() {
        return dom;
    }

    public Element createRoot(String name) {
        Element el = getW3CDocument().createElementNS(getRootElementNamespace(), name);
        getW3CDocument().appendChild(el);
        return el;
    }

    public abstract String getRootElementNamespace();
    public abstract DOMElement getRoot(XPath xpath);
    public abstract DOM copy();

}
