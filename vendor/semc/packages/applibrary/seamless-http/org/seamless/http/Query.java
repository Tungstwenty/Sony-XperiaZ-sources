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
package org.seamless.http;

import java.io.UnsupportedEncodingException;
import java.net.URL;
import java.net.URLDecoder;
import java.util.ArrayList;
import java.util.Arrays;
import java.util.Collections;
import java.util.Enumeration;
import java.util.HashMap;
import java.util.LinkedHashMap;
import java.util.List;
import java.util.Map;
import java.util.TreeMap;

/**
 * Parses query strings into multi-valued hashmaps. This should be part of the JDK.
 * <p>
 *     TODO: Can't extend LinkedHashMap (or was it TreeMap?), broken in GWT RPC serialization!
 * </p>
 * @author Christian Bauer
 */
public class Query {

    final protected Map<String, List<String>> parameters = new LinkedHashMap<String, List<String>>();

    public static Query newInstance(Map<String, List<String>> parameters) {
        Query query = new Query();
        query.parameters.putAll(parameters);
        return query;
    }

    public Query() {
    }

    public Query(Map<String, String[]> parameters) {
        for (Map.Entry<String, String[]> entry : parameters.entrySet()) {
            List<String> list =
                Arrays.asList(entry.getValue() != null ? entry.getValue() : new String[0]);
            this.parameters.put(entry.getKey(), list);
        }
    }

    public Query(URL url) {
        this(url.getQuery());
    }

    public Query(String qs) {
        if (qs == null) return;

        // Parse query string
        String pairs[] = qs.split("&");
        for (String pair : pairs) {
            String name;
            String value;
            int pos = pair.indexOf('=');
            // for "n=", the value is "", for "n", the value is null
            if (pos == -1) {
                name = pair;
                value = null;
            } else {
                try {
                    name = URLDecoder.decode(pair.substring(0, pos), "UTF-8");
                    value = URLDecoder.decode(pair.substring(pos + 1, pair.length()), "UTF-8");
                } catch (UnsupportedEncodingException e) {
                    // Not really possible, throw unchecked
                    throw new IllegalStateException("Query string is not UTF-8");
                }
            }
            List<String> list = parameters.get(name);
            if (list == null) {
                list = new ArrayList<String>();
                parameters.put(name, list);
            }
            list.add(value);
        }
    }

    public String get(String name) {
        List<String> values = parameters.get(name);
        if (values == null)
            return "";

        if (values.size() == 0)
            return "";

        return values.get(0);
    }

    public String[] getValues(String name) {
        List<String> values = parameters.get(name);
        if (values == null)
            return null;

        return values.toArray(new String[values.size()]);
    }

    public List<String> getValuesAsList(String name) {
        return parameters.containsKey(name)
            ? Collections.unmodifiableList(parameters.get(name))
            : null;
    }

    public Enumeration<String> getNames() {
        return Collections.enumeration(parameters.keySet());
    }

    public Map<String, String[]> getMap() {
        Map<String, String[]> map = new TreeMap<String, String[]>();
        for (Map.Entry<String, List<String>> entry : parameters.entrySet()) {
            List<String> list = entry.getValue();
            String[] values;
            if (list == null)
                values = null;
            else
                values = list.toArray(new String[list.size()]);
            map.put(entry.getKey(), values);
        }
        return map;
    }

    public Map<String, List<String>> getMapWithLists() {
        return Collections.unmodifiableMap(parameters);
    }

    public boolean isEmpty() {
        return parameters.size() == 0;
    }

    public Query cloneAndAdd(String name, String... values) {
        Map<String, List<String>> params = new HashMap(getMapWithLists());
        List<String> existingValues = params.get(name);
        if (existingValues == null) {
            existingValues = new ArrayList<String>();
            params.put(name, existingValues);
        }
        existingValues.addAll(Arrays.asList(values));
        return Query.newInstance(params);
    }

    @Override
    public String toString() {
        StringBuilder sb = new StringBuilder();
        for (Map.Entry<String, List<String>> entry : parameters.entrySet()) {
            for (String v : entry.getValue()) {
                if (v == null || v.length() == 0) continue;
                if (sb.length() > 0) sb.append("&");
                sb.append(entry.getKey());
                sb.append("=");
                sb.append(v);
            }
        }
        return sb.toString();
    }
}