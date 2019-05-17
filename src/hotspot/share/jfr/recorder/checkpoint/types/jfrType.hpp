/*
 * Copyright (c) 2016, 2019, Oracle and/or its affiliates. All rights reserved.
 * DO NOT ALTER OR REMOVE COPYRIGHT NOTICES OR THIS FILE HEADER.
 *
 * This code is free software; you can redistribute it and/or modify it
 * under the terms of the GNU General Public License version 2 only, as
 * published by the Free Software Foundation.
 *
 * This code is distributed in the hope that it will be useful, but WITHOUT
 * ANY WARRANTY; without even the implied warranty of MERCHANTABILITY or
 * FITNESS FOR A PARTICULAR PURPOSE.  See the GNU General Public License
 * version 2 for more details (a copy is included in the LICENSE file that
 * accompanied this code).
 *
 * You should have received a copy of the GNU General Public License version
 * 2 along with this work; if not, write to the Free Software Foundation,
 * Inc., 51 Franklin St, Fifth Floor, Boston, MA 02110-1301 USA.
 *
 * Please contact Oracle, 500 Oracle Parkway, Redwood Shores, CA 94065 USA
 * or visit www.oracle.com if you need additional information or have any
 * questions.
 *
 */

#ifndef SHARE_JFR_RECORDER_CHECKPOINT_TYPES_JFRTYPE_HPP
#define SHARE_JFR_RECORDER_CHECKPOINT_TYPES_JFRTYPE_HPP

#include "jfr/metadata/jfrSerializer.hpp"

class JfrThreadConstantSet : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class JfrThreadGroupConstant : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class ClassUnloadTypeSet : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class FlushTypeSet : public JfrSerializer {
 size_t _elements;
 public:
  void serialize(JfrCheckpointWriter& writer);
  size_t elements() const;
};
class FlagValueOriginConstant : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class MonitorInflateCauseConstant : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class GCCauseConstant : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class GCNameConstant : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class GCWhenConstant : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class G1HeapRegionTypeConstant : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class GCThresholdUpdaterConstant : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class MetadataTypeConstant : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class MetaspaceObjectTypeConstant : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class G1YCTypeConstant : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class ReferenceTypeConstant : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class NarrowOopModeConstant : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class CompilerPhaseTypeConstant : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class CodeBlobTypeConstant : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class VMOperationTypeConstant : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class TypeSet : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class ThreadStateConstant : public JfrSerializer {
 public:
  void serialize(JfrCheckpointWriter& writer);
};

class JfrThreadConstant : public JfrSerializer {
 private:
  Thread* _thread;
 public:
  JfrThreadConstant(Thread* t) : _thread(t) {}
  void serialize(JfrCheckpointWriter& writer);
};

#endif // SHARE_JFR_RECORDER_CHECKPOINT_TYPES_JFRTYPE_HPP
