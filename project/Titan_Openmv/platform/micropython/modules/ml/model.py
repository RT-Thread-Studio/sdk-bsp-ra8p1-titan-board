# Copyright (C) 2024 OpenMV, LLC.
#
# Redistribution and use in source and binary forms, with or without
# modification, are permitted provided that the following conditions
# are met:
#
# 1. Redistributions of source code must retain the above copyright
#    notice, this list of conditions and the following disclaimer.
# 2. Redistributions in binary form must reproduce the above copyright
#    notice, this list of conditions and the following disclaimer in
#    the documentation and/or other materials provided with the
#    distribution.
# 3. Any redistribution, use, or modification in source or binary form
#    is done solely for personal benefit and not for any commercial
#    purpose or for monetary gain. For commercial licensing options,
#    please contact openmv@openmv.io
#
# THIS SOFTWARE IS PROVIDED BY THE LICENSOR AND COPYRIGHT OWNER "AS IS"
# AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
# THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR
# PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE LICENSOR OR COPYRIGHT
# OWNER BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL,
# EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO,
# PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS OF USE, DATA, OR
# PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY THEORY
# OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT
# (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE
# OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
import uml

Workspace = uml.Workspace

def _default_workspace(kwargs):
    # Explicit None preserves ordinary allocation; custom callbacks keep their
    # independent arenas and existing reentrant behaviour by default.
    if "workspace" in kwargs:
        value = kwargs["workspace"]
        if value is not None and type(value) is not Workspace:
            raise TypeError("workspace must be Workspace or None")
        return value
    post = kwargs.get("postprocess", None)
    if type(post).__name__ not in ("BlazePalm", "HandLandmarks"):
        return None
    from ml.postprocessing.mediapipe import BlazePalm, HandLandmarks
    if type(post) is BlazePalm or type(post) is HandLandmarks:
        # Internal policy request. Native code still requires a reviewed SHA256.
        return True
    return None

import image
from ml.preprocessing import Normalization


class Model(uml.Model):
    def __init__(self, *args, **kwargs):
        super().__init__(*args, kwargs.get("postprocess", None), workspace=_default_workspace(kwargs))
        try:
            path = args[0].split(".")[0] + ".txt"
            self.labels = [line.rstrip('\n') for line in open(path, "r")]
        except Exception:
            self.labels = None

    def predict(self, args, **kwargs):
        args = [Normalization()(x) if isinstance(x, image.Image) else x for x in args]
        return super().predict(args, **kwargs)
