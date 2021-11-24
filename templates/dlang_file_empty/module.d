{% load kdev_filters %}
{% block license_header %}
{% if license %}
/*
{{ license|lines_prepend:" * " }}
 */
{% endif %}
{% endblock license_header %}

module {{ name }};

// Write your module code here
