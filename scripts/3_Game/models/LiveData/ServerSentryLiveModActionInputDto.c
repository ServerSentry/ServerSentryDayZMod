/**
	\file ServerSentryLiveModActionInputDto.c
	\brief Defines the Live mod action input DTO live-data transfer model.
*/

/**
	\brief Describes the key schema used by a map-style action input.
*/
class ServerSentryLiveModActionMapKeyDto
{
    /**
    	\brief Logical value type used for schema validation.
    */
    string valueType;
    /**
    	\brief Human-readable label shown to users.
    */
    string label;
    /**
    	\brief Placeholder shown by the website input control.
    */
    string placeholder;
    /**
    	\brief Help text shown next to the website input control.
    */
    string helpText;
    /**
    	\brief Allowed values for a select-style input.
    */
    ref array<string> options;
    /**
    	\brief Minimum accepted string length.
    */
    int minLength;
    /**
    	\brief Maximum accepted string length.
    */
    int maxLength;
    /**
    	\brief Validation pattern applied to the string value.
    */
    string pattern;

    /**
    	\brief Initializes a new ServerSentryLiveModActionMapKeyDto instance.
    */
    void ServerSentryLiveModActionMapKeyDto()
    {
        valueType = "string";
        label = "Key";
        placeholder = "";
        helpText = "";
        options = new array<string>();
        minLength = 0;
        maxLength = 0;
        pattern = "";
    }
}

/**
	\brief Describes one website input and its validation schema.
*/
class ServerSentryLiveModActionInputDto
{
    // Flat input metadata used by simple website controls.
    /**
    	\brief Stable key used for lookup and serialization.
    */
    string key;
    /**
    	\brief Input or event type exposed to external consumers.
    */
    string type;
    /**
    	\brief Human-readable label shown to users.
    */
    string label;
    /**
    	\brief Indicates whether the value must be supplied.
    */
    bool required;
    /**
    	\brief Placeholder shown by the website input control.
    */
    string placeholder;
    /**
    	\brief Help text shown next to the website input control.
    */
    string helpText;
    /**
    	\brief Default scalar value used by the input.
    */
    string defaultValue;
    /**
    	\brief Minimum accepted numeric value.
    */
    float min;
    /**
    	\brief Maximum accepted numeric value.
    */
    float max;
    /**
    	\brief Increment used by a numeric input control.
    */
    float step;
    /**
    	\brief Allowed values for a select-style input.
    */
    ref array<string> options;

    // Typed schema metadata used by nested inputs and validation.
    /**
    	\brief Logical value type used for schema validation.
    */
    string valueType;
    /**
    	\brief Preferred website widget used to edit the value.
    */
    string widget;
    /**
    	\brief Default structured value encoded as JSON.
    */
    string defaultJson;
    /**
    	\brief Minimum accepted number of list items.
    */
    int minItems;
    /**
    	\brief Maximum accepted number of list items.
    */
    int maxItems;
    /**
    	\brief Minimum accepted number of map entries.
    */
    int minEntries;
    /**
    	\brief Maximum accepted number of map entries.
    */
    int maxEntries;
    /**
    	\brief Schema used for each list item.
    */
    ref ServerSentryLiveModActionInputDto item;
    /**
    	\brief Schemas contained by an object input.
    */
    ref array<ref ServerSentryLiveModActionInputDto> fields;
    /**
    	\brief Schema used for map keys.
    */
    ref ServerSentryLiveModActionMapKeyDto mapKey;
    /**
    	\brief Schema used for map values.
    */
    ref ServerSentryLiveModActionInputDto mapValue;

    /**
    	\brief Initializes a new ServerSentryLiveModActionInputDto instance.
    */
    void ServerSentryLiveModActionInputDto()
    {
        key = "";
        type = "text";
        label = "Input";
        required = false;
        placeholder = "";
        helpText = "";
        defaultValue = "";
        min = 0;
        max = 0;
        step = 0;
        options = new array<string>();

        valueType = "string";
        widget = "text";
        defaultJson = "";
        minItems = 0;
        maxItems = 0;
        minEntries = 0;
        maxEntries = 0;
        item = null;
        fields = new array<ref ServerSentryLiveModActionInputDto>();
        mapKey = null;
        mapValue = null;
    }


}

/**
	\brief Creates input descriptors for ServerSentry live actions.
*/
class ServerSentryLiveModActionInputFactory
{
    /**
    	\brief Creates a single-line text input descriptor.
    	\param key \p stable key used for lookup or serialization.
    	\param label \p human-readable label shown to users.
    	\param required \p true when the value must be supplied.
    	\param placeholder \p placeholder shown by the website input control.
    	\param helpText \p help text shown next to the website input control.
    	\return \p The resulting live mod action input DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionInputDto Text(string key, string label, bool required = false, string placeholder = "", string helpText = "")
    {
        ServerSentryLiveModActionInputDto input = Base(key, label, "string", "text", required);
        input.type = "text";
        input.placeholder = placeholder;
        input.helpText = helpText;
        return input;
    }

    /**
    	\brief Creates a multi-line text input descriptor.
    	\param key \p stable key used for lookup or serialization.
    	\param label \p human-readable label shown to users.
    	\param required \p true when the value must be supplied.
    	\param placeholder \p placeholder shown by the website input control.
    	\param helpText \p help text shown next to the website input control.
    	\return \p The resulting live mod action input DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionInputDto TextArea(string key, string label, bool required = false, string placeholder = "", string helpText = "")
    {
        ServerSentryLiveModActionInputDto input = Base(key, label, "string", "textarea", required);
        input.type = "textarea";
        input.placeholder = placeholder;
        input.helpText = helpText;
        return input;
    }

    /**
    	\brief Creates a floating-point input descriptor.
    	\param key \p stable key used for lookup or serialization.
    	\param label \p human-readable label shown to users.
    	\param required \p true when the value must be supplied.
    	\param min \p minimum accepted numeric value.
    	\param max \p maximum accepted numeric value.
    	\param step \p increment used by the numeric input control.
    	\param defaultValue \p default value used when no explicit value is supplied.
    	\param helpText \p help text shown next to the website input control.
    	\return \p The resulting live mod action input DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionInputDto Number(string key, string label, bool required = false, float min = 0, float max = 0, float step = 1, string defaultValue = "", string helpText = "")
    {
        ServerSentryLiveModActionInputDto input = Base(key, label, "number", "number", required);
        input.type = "number";
        input.min = min;
        input.max = max;
        input.step = step;
        input.defaultValue = defaultValue;
        input.helpText = helpText;
        return input;
    }

    /**
    	\brief Creates an integer input descriptor.
    	\param key \p stable key used for lookup or serialization.
    	\param label \p human-readable label shown to users.
    	\param required \p true when the value must be supplied.
    	\param min \p minimum accepted numeric value.
    	\param max \p maximum accepted numeric value.
    	\param step \p increment used by the numeric input control.
    	\param defaultValue \p default value used when no explicit value is supplied.
    	\param helpText \p help text shown next to the website input control.
    	\return \p The resulting live mod action input DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionInputDto Integer(string key, string label, bool required = false, int min = 0, int max = 0, int step = 1, string defaultValue = "", string helpText = "")
    {
        ServerSentryLiveModActionInputDto input = Base(key, label, "integer", "number", required);
        input.type = "number";
        input.min = min;
        input.max = max;
        input.step = step;
        input.defaultValue = defaultValue;
        input.helpText = helpText;
        return input;
    }

    /**
    	\brief Creates a boolean checkbox input descriptor.
    	\param key \p stable key used for lookup or serialization.
    	\param label \p human-readable label shown to users.
    	\param defaultValue \p default value used when no explicit value is supplied.
    	\param helpText \p help text shown next to the website input control.
    	\return \p The resulting live mod action input DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionInputDto Boolean(string key, string label, bool defaultValue = false, string helpText = "")
    {
        ServerSentryLiveModActionInputDto input = Base(key, label, "boolean", "checkbox", false);
        input.type = "checkbox";
        input.defaultValue = "false";
        if (defaultValue)
        {
            input.defaultValue = "true";
        }
        input.helpText = helpText;
        return input;
    }

    /**
    	\brief Creates the standard remove-product checkbox descriptor.
    	\param label \p human-readable label shown to users.
    	\param helpText \p help text shown next to the website input control.
    	\return \p The resulting live mod action input DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionInputDto RemoveProduct(string label = "Remove product effect", string helpText = "Removes the effect created by this product instead of applying it.")
    {
        return Boolean("removeProduct", label, false, helpText);
    }

    /**
    	\brief Creates an RGB color-picker input descriptor.
    	\param key \p stable key used for lookup or serialization.
    	\param label \p human-readable label shown to users.
    	\param required \p true when the value must be supplied.
    	\param defaultValue \p default value used when no explicit value is supplied.
    	\param helpText \p help text shown next to the website input control.
    	\return \p The resulting live mod action input DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionInputDto Color(string key, string label, bool required = false, string defaultValue = "#FFFFFF", string helpText = "RGB color in #RRGGBB format.")
    {
        ServerSentryLiveModActionInputDto input = Base(key, label, "string", "color", required);
        input.type = "color";
        input.placeholder = defaultValue;
        input.defaultValue = defaultValue;
        input.helpText = helpText;
        return input;
    }

    /**
    	\brief Creates a time input descriptor.
    	\param key \p stable key used for lookup or serialization.
    	\param label \p human-readable label shown to users.
    	\param required \p true when the value must be supplied.
    	\param defaultValue \p default value used when no explicit value is supplied.
    	\return \p The resulting live mod action input DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionInputDto Time(string key, string label, bool required = false, string defaultValue = "")
    {
        ServerSentryLiveModActionInputDto input = Base(key, label, "string", "text", required);
        input.type = "time";
        input.defaultValue = defaultValue;
        return input;
    }

    /**
    	\brief Creates a select input descriptor with fixed options.
    	\param key \p stable key used for lookup or serialization.
    	\param label \p human-readable label shown to users.
    	\param options \p allowed values for the input.
    	\param required \p true when the value must be supplied.
    	\param defaultValue \p default value used when no explicit value is supplied.
    	\return \p The resulting live mod action input DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionInputDto Select(string key, string label, array<string> options, bool required = false, string defaultValue = "")
    {
        ServerSentryLiveModActionInputDto input = Base(key, label, "string", "select", required);
        input.type = "select";
        input.defaultValue = defaultValue;
        if (options)
        {
            foreach (string option: options)
            {
                input.options.Insert(option);
            }
        }
        return input;
    }

    /**
    	\brief Creates a list input descriptor.
    	\param key \p stable key used for lookup or serialization.
    	\param label \p human-readable label shown to users.
    	\param item \p item or value processed by the operation.
    	\param required \p true when the value must be supplied.
    	\param minItems \p minimum number of accepted list items.
    	\param maxItems \p maximum number of accepted list items.
    	\param defaultJson \p default value encoded as JSON.
    	\return \p The resulting live mod action input DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionInputDto List(string key, string label, ServerSentryLiveModActionInputDto item, bool required = false, int minItems = 0, int maxItems = 200, string defaultJson = "[]")
    {
        ServerSentryLiveModActionInputDto input = Base(key, label, "array", "list", required);
        input.type = "array";
        input.item = item;
        input.minItems = minItems;
        input.maxItems = maxItems;
        input.defaultJson = defaultJson;
        return input;
    }

    /**
    	\brief Creates a nested object input descriptor.
    	\param key \p stable key used for lookup or serialization.
    	\param label \p human-readable label shown to users.
    	\param fields \p field descriptors contained by the object input.
    	\param required \p true when the value must be supplied.
    	\param defaultJson \p default value encoded as JSON.
    	\return \p The resulting live mod action input DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionInputDto Object(string key, string label, array<ref ServerSentryLiveModActionInputDto> fields, bool required = false, string defaultJson = "{}")
    {
        ServerSentryLiveModActionInputDto input = Base(key, label, "object", "group", required);
        input.type = "object";
        input.defaultJson = defaultJson;
        if (fields)
        {
            foreach (ServerSentryLiveModActionInputDto field: fields)
            {
                input.fields.Insert(field);
            }
        }
        return input;
    }

    /**
    	\brief Creates a map input descriptor.
    	\param key \p stable key used for lookup or serialization.
    	\param label \p human-readable label shown to users.
    	\param mapKey \p schema that describes map keys.
    	\param mapValue \p schema that describes map values.
    	\param required \p true when the value must be supplied.
    	\param minEntries \p minimum number of accepted map entries.
    	\param maxEntries \p maximum number of accepted map entries.
    	\param defaultJson \p default value encoded as JSON.
    	\return \p The resulting live mod action input DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionInputDto MapInput(string key, string label, ServerSentryLiveModActionMapKeyDto mapKey, ServerSentryLiveModActionInputDto mapValue, bool required = false, int minEntries = 0, int maxEntries = 200, string defaultJson = "{}")
    {
        ServerSentryLiveModActionInputDto input = Base(key, label, "map", "list", required);
        input.type = "map";
        input.mapKey = mapKey;
        input.mapValue = mapValue;
        input.minEntries = minEntries;
        input.maxEntries = maxEntries;
        input.defaultJson = defaultJson;
        return input;
    }

    /**
    	\brief Creates a raw JSON input descriptor.
    	\param key \p stable key used for lookup or serialization.
    	\param label \p human-readable label shown to users.
    	\param required \p true when the value must be supplied.
    	\param defaultJson \p default value encoded as JSON.
    	\return \p The resulting live mod action input DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionInputDto Json(string key, string label, bool required = false, string defaultJson = "{}")
    {
        ServerSentryLiveModActionInputDto input = Base(key, label, "json", "json", required);
        input.type = "json";
        input.defaultJson = defaultJson;
        return input;
    }

    /**
    	\brief Creates a string map-key descriptor.
    	\param label \p human-readable label shown to users.
    	\param placeholder \p placeholder shown by the website input control.
    	\return \p The resulting live mod action map key DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionMapKeyDto StringMapKey(string label = "Key", string placeholder = "")
    {
        ServerSentryLiveModActionMapKeyDto key = new ServerSentryLiveModActionMapKeyDto();
        key.valueType = "string";
        key.label = label;
        key.placeholder = placeholder;
        return key;
    }

    /**
    	\brief Creates an integer map-key descriptor.
    	\param label \p human-readable label shown to users.
    	\return \p The resulting live mod action map key DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionMapKeyDto IntegerMapKey(string label = "Key")
    {
        ServerSentryLiveModActionMapKeyDto key = new ServerSentryLiveModActionMapKeyDto();
        key.valueType = "integer";
        key.label = label;
        return key;
    }

    /**
    	\brief Creates a map-key descriptor restricted to fixed string options.
    	\param label \p human-readable label shown to users.
    	\param options \p allowed values for the input.
    	\return \p The resulting live mod action map key DTO instance, or null when unavailable.
    */
    static ServerSentryLiveModActionMapKeyDto EnumMapKey(string label, array<string> options)
    {
        ServerSentryLiveModActionMapKeyDto key = new ServerSentryLiveModActionMapKeyDto();
        key.valueType = "enum";
        key.label = label;
        if (options)
        {
            foreach (string option: options)
            {
                key.options.Insert(option);
            }
        }
        return key;
    }

    /**
    	\brief Creates a base input descriptor shared by specialized factories.
    	\param key \p stable key used for lookup or serialization.
    	\param label \p human-readable label shown to users.
    	\param valueType \p value type used by the operation.
    	\param widget \p widget used by the operation.
    	\param required \p true when the value must be supplied.
    	\return \p The resulting live mod action input DTO instance, or null when unavailable.
    */
    protected static ServerSentryLiveModActionInputDto Base(string key, string label, string valueType, string widget, bool required)
    {
        ServerSentryLiveModActionInputDto input = new ServerSentryLiveModActionInputDto();
        input.key = key;
        input.label = label;
        input.valueType = valueType;
        input.widget = widget;
        input.required = required;
        return input;
    }
}
