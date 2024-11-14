#include <json-c/json.h>
#include <stdio.h>

int main() {
    struct json_object *jobj = json_object_new_object();
    json_object_object_add(jobj, "message", json_object_new_string("Hello, json-c!"));
    printf("JSON Output: %s\n", json_object_to_json_string(jobj));
    
    json_object_put(jobj);  // Free JSON object
    return 0;
}
