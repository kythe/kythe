"""A macro for consistent extra_action plumbing.

The extractor_action macro plumbs together a Kythe extractor to an extra_action
with an associated action_listener.
"""

def extractor_action(name, extractor, args, mnemonics, output, needs_output = False, data = [], tags = [], env = []):
    """Creates an extra_action and an associated action_listener.

    The action_listener is given the specified name and mnemonics.
    The extra_action invokes extractor with the given list of args, output
    template, and (optional) data dependencies.

    If the name of the action_listener is "foo", the associated extra_action will
    be named "foo_extra_action".

    Args:
      name: The name of the action_listener.
      extractor: The label of the extractor binary.
      args: The argument list for the extractor binary, not including the command.
      needs_output: Whether the extra action requires the action's output.
      mnemonics: The list of action mnemonics to shadow.
      output: The output file template (string).
      data: A list of data dependencies (optional).
      tags: A list of build tags (optional).
      env: A list of environment variable KEY=VALUE strings (optional).
    """
    xa_name = name + "_extra_action"

    native.extra_action(
        name = xa_name,
        data = data,
        out_templates = [output],
        tools = [extractor],
        cmd = " ".join(env + ["$(location %s)" % extractor] + args),
        tags = tags,
        requires_action_output = needs_output,
    )

    native.action_listener(
        name = name,
        extra_actions = [":" + xa_name],
        mnemonics = mnemonics,
        visibility = ["//visibility:public"],
    )
