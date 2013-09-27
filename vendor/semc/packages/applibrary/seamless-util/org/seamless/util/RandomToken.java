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
package org.seamless.util;

import java.security.SecureRandom;
import java.util.Random;

/**
 * @author Christian Bauer
 */
public class RandomToken {

    final protected Random random;

    public RandomToken() {
        try {
            random = SecureRandom.getInstance("SHA1PRNG", "SUN");
        } catch (Exception ex) {
            throw new RuntimeException(ex);
        }
        // Force secure seeding
        random.nextBytes(new byte[1]);
    }

    public String generate() {
        String token = null;
        while (token == null || token.length() == 0) {
            long r0 = random.nextLong();
            if (r0 < 0)
                r0 = -r0;
            long r1 = random.nextLong();
            if (r1 < 0)
                r1 = -r1;
            token = Long.toString(r0, 36) + Long.toString(r1, 36);
        }
        return token;
    }

}
